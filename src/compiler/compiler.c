#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "lexer/lexer.h"
#include "compiler/bytecode.h"
#include "compiler/compiler.h"
#include "compiler/instruction.h"
#include "utils/error.h"
#include "utils/common.h"

static bool DEBUG = false;

compiler_t compiler;

static void emit(byte_t instruction, int line) {
    bytecode_add(compiler.bytecode, instruction, line);
}

static void emit_numeric_literal(char* raw_value, int line) {
    emit(OP_LOAD_CONST, line);

    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = atof(raw_value);

    uint16_t value_index = pool_add(compiler.pool, value);
    
    byte_t* bytes = uint16_to_bytes(value_index);
    for (int i = 0; i < 2; i++) {
        emit(bytes[i], line);
    }
}

static void emit_numeric_literal_num(double numeric_value, int line) {
    emit(OP_LOAD_CONST, line);

    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = numeric_value;

    uint16_t value_index = pool_add(compiler.pool, value);
    
    byte_t* bytes = uint16_to_bytes(value_index);
    for (int i = 0; i < 2; i++) {
        emit(bytes[i], line);
    }
}

static void emit_boolean_literal(char* raw_value, int line) {
    emit(OP_LOAD_CONST, line);

    value_t value;
    value.type = TYPE_BOOLEAN;
    value.as.boolean = strcmp(raw_value, "true") == 0;

    uint16_t value_index = pool_add(compiler.pool, value);
    
    byte_t* bytes = uint16_to_bytes(value_index);
    for (int i = 0; i < 2; i++) {
        emit(bytes[i], line);
    }
}

static void emit_string_literal(char* raw_value, int line) {
    emit(OP_LOAD_CONST, line);

    value_t value;
    value.type = TYPE_STRING;
    value.as.string = raw_value;

    uint16_t value_index = pool_add(compiler.pool, value);
    
    byte_t* bytes = uint16_to_bytes(value_index);
    for (int i = 0; i < 2; i++) {
        emit(bytes[i], line);
    }
}

static double get_main_func_ip() {
    for (int i = 0; i < compiler.bytecode->count; i++) {
        if (compiler.bytecode->instructions[i] == OP_LOAD_CONST) {
            i++;

            byte_t bytes[2];
            for (int j = 0; j < 2; j++) {
                bytes[j] = compiler.bytecode->instructions[i + j];
            }

            i += 2;

            uint16_t index = bytes_to_uint16(bytes);
            value_t* value = pool_get(compiler.pool, index);

            if (value == NULL) {
                continue;
            }

            if (value->type == TYPE_STRING && strcmp(value->as.string, "main") == 0 && compiler.bytecode->instructions[i] == OP_FUNC_DEF) {
                return (double)(i + 1);
            }

            i -= 1;
        }
    }

    error_throw(ERROR_COMPILER, "main() function is missing", 0);
    return 0;
}

static void emit_main_func_call() {
    double main_func_ip = get_main_func_ip();
    emit_numeric_literal_num(main_func_ip, 0);
    emit_numeric_literal_num(0, 0);
    emit(OP_CALL, 0);
}

static void compile_var_declaration();
static void compile_func_declaration();
static void compile_func_declaration_param_list();
static void compile_func_declaration_body();
static void compile_object_declaration();
static void compile_object_declaration_body();
static void compile_object_declaration_property();
static void compile_array_instantiation_expression();
static void compile_conditional_statement();
static void compile_while_statement();
static void compile_call_statement();
static void compile_return();
static void compile_print();

static void compile_expression();
static void compile_object_instantiation_expression();
static void compile_logical_expression();
static void compile_relational_expression();
static void compile_additive_expression();
static void compile_multiplicative_expression();
static void compile_access();
static void compile_call_expression();
static double compile_call_expression_args();
static void compile_primary_expression();
static void compile_identifier();
static void compile_numeric_literal();
static void compile_boolean_literal();
static void compile_string_literal();

static void compile_parenthesised_expression();

static void compile_logical_operator(token_t operator_token);
static void compile_relational_operator(token_t operator_token);
static void compile_binary_operator(token_t operator_token);

static token_t peek() {
    return compiler.current_token;
}

static token_t advance() {
    token_t current_token = compiler.current_token;
    compiler.current_token = lexer_get_token();
    return current_token;
}

static token_t assert(token_type type) {
    token_t token = advance();

    if (token.type != type) {
        error_throw(ERROR_COMPILER, "token assertion failed", token.line);
    }

    return token;
}

void compiler_init() {
    compiler.bytecode = bytecode_init();
    compiler.current_token = lexer_get_token();
    compiler.pool = pool_init(50);
}

bytecode_t* compile() {
    while (peek().type != TOKEN_EOF) {
        switch (peek().type) {
            case TOKEN_VAR:
                compile_var_declaration();
                break;
            case TOKEN_FUNC:
                compile_func_declaration();
                break;
            case TOKEN_OBJECT:
                compile_object_declaration();
                break;
            default:
                error_throw(ERROR_COMPILER, "Unrecognized top level statement", peek().line);
        }
    }

    emit_main_func_call();
    return compiler.bytecode;
}

pool_t* compiler_get_pool() {
    return compiler.pool;
}

static void compile_var_declaration() {
    if (DEBUG == true) printf("Compiling compile_var_declaration\n");

    int line = assert(TOKEN_VAR).line;
    token_t identifier_token = assert(TOKEN_IDENTIFIER);

    if (peek().type == TOKEN_SEMICOLON) {
        emit_numeric_literal_num(0, identifier_token.line);
    } else {
        assert(TOKEN_ASSIGNMENT);
        compile_expression();
    }

    assert(TOKEN_SEMICOLON);

    emit_string_literal(substring(identifier_token.start, identifier_token.length), line);
    emit(OP_STORE_VAR, line);
}

static void compile_func_declaration() {
    if (DEBUG == true) printf("Compiling compile_func_declaration\n");

    int line = assert(TOKEN_FUNC).line;
    token_t identifier_token = assert(TOKEN_IDENTIFIER);

    emit_string_literal(substring(identifier_token.start, identifier_token.length), line);
    emit(OP_FUNC_DEF, line);

    assert(TOKEN_OPEN_PAREN);
    compile_func_declaration_param_list();
    assert(TOKEN_CLOSE_PAREN);

    assert(TOKEN_OPEN_BRACE);
    compile_func_declaration_body();
    line = assert(TOKEN_CLOSE_BRACE).line;

    // explicitely emit 0 return
    emit_numeric_literal_num(0, line);
    emit(OP_RETURN, line);

    emit(OP_FUNC_END, line);
}

static void compile_func_declaration_param_list() {
    if (DEBUG == true) printf("Compiling compile_func_declaration_param_list\n");

    while (peek().type != TOKEN_CLOSE_PAREN) {
        int line = assert(TOKEN_VAR).line;
        token_t identifier_token = assert(TOKEN_IDENTIFIER);

        emit_string_literal(substring(identifier_token.start, identifier_token.length), line);
        emit(OP_STORE_VAR, line);

        if (peek().type == TOKEN_COMMA) {
            advance();
        }
    }
}

static void compile_func_declaration_body() {
    if (DEBUG == true) printf("Compiling compile_func_declaration_body\n");

    while (peek().type != TOKEN_CLOSE_BRACE) {
        switch (peek().type) {
            case TOKEN_VAR: {
                compile_var_declaration();
                break;
            }
            case TOKEN_IF: {
                compile_conditional_statement();
                break;
            }
            case TOKEN_WHILE: {
                compile_while_statement();
                break;
            }
            case TOKEN_PRINT: {
                compile_print();
                break;
            }
            case TOKEN_RETURN: {
                compile_return();
                break;
            }
            case TOKEN_IDENTIFIER: {
                compile_call_statement();
                break;
            }
            default: {
                error_throw(ERROR_COMPILER, "Unknown statement in function body", peek().line);
                break;
            }
        }
    }
}

static void compile_object_declaration() {
    if (DEBUG == true) printf("Compiling compile_object_declaration\n");

    int line = assert(TOKEN_OBJECT).line;
    token_t identifier_token = assert(TOKEN_IDENTIFIER);

    emit_string_literal(substring(identifier_token.start, identifier_token.length), line);
    emit(OP_OBJ_DEF, line);

    assert(TOKEN_OPEN_BRACE);
    compile_object_declaration_body();
    line = assert(TOKEN_CLOSE_BRACE).line;
    emit(OP_OBJ_END, line);
}

static void compile_object_declaration_body() {
    if (DEBUG == true) printf("Compiling compile_object_declaration_body\n");

    while (peek().type != TOKEN_CLOSE_BRACE) {
        switch (peek().type) {
            case TOKEN_VAR: {
                compile_object_declaration_property();
                break;
            }
            default: {
                return error_throw(ERROR_RUNTIME, "Unknown statement in object declaration body", peek().line);
            }
        }
    }
}

static void compile_object_declaration_property() {
    if (DEBUG == true) printf("Compiling compile_object_declaration_property\n");

    int line = assert(TOKEN_VAR).line;
    token_t identifier_token = assert(TOKEN_IDENTIFIER);

    if (peek().type == TOKEN_SEMICOLON) {
        emit_numeric_literal_num(0, identifier_token.line);
    } else {
        assert(TOKEN_ASSIGNMENT);
        compile_expression();
    }

    assert(TOKEN_SEMICOLON);

    emit_string_literal(substring(identifier_token.start, identifier_token.length), line);
    emit(OP_STORE_PROP, line);
}

static inline void update_jump_values(int source_ip, int jump_ip) {
    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = jump_ip;

    // TODO: don't store jump ips into constant pool, rather a separate pool
    uint16_t value_index = pool_add(compiler.pool, value);
    
    byte_t* bytes = uint16_to_bytes(value_index);
    compiler.bytecode->instructions[source_ip + 1] = bytes[0];
    compiler.bytecode->instructions[source_ip + 2] = bytes[1];
}

static void compile_conditional_statement() {
    if (DEBUG == true) printf("Compiling compile_conditional_statement\n");

    int line = assert(TOKEN_IF).line;
    assert(TOKEN_OPEN_PAREN);
    compile_expression();

    int ip1 = compiler.bytecode->count;
    // dummy value 0
    emit_numeric_literal_num(0, line);
    emit(OP_JUMP_IF_FALSE, line);

    assert(TOKEN_CLOSE_PAREN);
    assert(TOKEN_OPEN_BRACE);

    while (peek().type != TOKEN_CLOSE_BRACE) {
        compile_func_declaration_body();
    }

    line = assert(TOKEN_CLOSE_BRACE).line;

    if (peek().type != TOKEN_ELSE) {
        int ip2 = compiler.bytecode->count;
        update_jump_values(ip1, ip2);
        return;
    }

    int ip3 = compiler.bytecode->count;
    // dummy value 0
    emit_numeric_literal_num(0, line);
    emit(OP_JUMP, line);

    int ip4 = compiler.bytecode->count;
    update_jump_values(ip1, ip4);

    assert(TOKEN_ELSE);
    assert(TOKEN_OPEN_BRACE);

    while (peek().type != TOKEN_CLOSE_BRACE) {
        compile_func_declaration_body();
    }

    assert(TOKEN_CLOSE_BRACE);

    int ip5 = compiler.bytecode->count;
    update_jump_values(ip3, ip5);
}

static void compile_while_statement() {
    if (DEBUG == true) printf("Compiling compile_while_statement\n");

    int line = assert(TOKEN_WHILE).line;
    assert(TOKEN_OPEN_PAREN);

    int ip1 = compiler.bytecode->count;

    compile_expression();

    int ip2 = compiler.bytecode->count;

    // dummy value 0
    emit_numeric_literal_num(0, line);
    emit(OP_JUMP_IF_FALSE, line);

    assert(TOKEN_CLOSE_PAREN);
    assert(TOKEN_OPEN_BRACE);

    while (peek().type != TOKEN_CLOSE_BRACE) {
        compile_func_declaration_body();
    }

    line = assert(TOKEN_CLOSE_BRACE).line;

    emit_numeric_literal_num((double)ip1, line);
    emit(OP_JUMP, line);

    int ip3 = compiler.bytecode->count;
    update_jump_values(ip2, ip3);
}

static void compile_call_statement() {
    if (DEBUG == true) printf("Compiling compile_call_statement\n");

    compile_call_expression();
    int line = assert(TOKEN_SEMICOLON).line;

    emit_numeric_literal_num(1, line);
    emit(OP_STACK_CLEAR, line);
}

static void compile_return() {
    if (DEBUG == true) printf("Compiling compile_return\n");

    int line = assert(TOKEN_RETURN).line;

    if (peek().type == TOKEN_SEMICOLON) {
        emit_numeric_literal_num(0, line);
    } else {
        compile_expression();
    }

    assert(TOKEN_SEMICOLON);
    emit(OP_RETURN, line);
}

static void compile_print() {
    if (DEBUG == true) printf("Compiling compile_print\n");

    int line = assert(TOKEN_PRINT).line;

    compile_expression();
    assert(TOKEN_SEMICOLON);

    emit(OP_PRINT, line);
}

// EXPRESSIONS

static void compile_expression() {
    if (DEBUG == true) printf("Compiling compile_expression\n");

    switch (peek().type) {
        case TOKEN_NEW: {
            compile_object_instantiation_expression();
            break;
        }
        case TOKEN_OPEN_BRACKET: {
            compile_array_instantiation_expression();
            break;
        }
        default: {
            compile_logical_expression();
            break;
        }
    }
}

static void compile_object_instantiation_expression() {
    int line = assert(TOKEN_NEW).line;
    token_t identifier_token = assert(TOKEN_IDENTIFIER);

    emit_string_literal(substring(identifier_token.start, identifier_token.length), identifier_token.line);
    emit(OP_NEW_OBJ, line);
}

static void compile_array_instantiation_expression() {
    int line = assert(TOKEN_OPEN_BRACKET).line;

    int count = 0;

    while (peek().type != TOKEN_CLOSE_BRACKET) {
        compile_expression();
        count++;

        if (peek().type == TOKEN_COMMA) {
            advance();
        }
    }

    assert(TOKEN_CLOSE_BRACKET);

    emit_numeric_literal_num((double)count, line);
    emit(OP_ARRAY_DEF, line);
}

static void compile_logical_expression() {
    if (DEBUG == true) printf("Compiling compile_logical_expression\n");

    compile_relational_expression();

    while (peek().type == TOKEN_AND || peek().type == TOKEN_OR) {
        token_t operator_token = advance();

        compile_relational_expression();
        compile_logical_operator(operator_token);
    }
}

static void compile_logical_operator(token_t operator_token) {
    if (DEBUG == true) printf("Compiling compile_logical_operator\n");

    switch (operator_token.type) {
        case TOKEN_AND:
            return emit(OP_AND, operator_token.line);
        case TOKEN_OR:
            return emit(OP_OR, operator_token.line);
        default:
            return error_throw(ERROR_COMPILER, "Unknown logical operator", operator_token.line);
    }
}

static void compile_relational_expression() {
    if (DEBUG == true) printf("Compiling compile_relational_expression\n");

    compile_additive_expression();

    if (peek().type == TOKEN_EQ || peek().type == TOKEN_NE || peek().type == TOKEN_GT || peek().type == TOKEN_GE || peek().type == TOKEN_LT || peek().type == TOKEN_LE) {
        token_t operator_token = advance();

        compile_additive_expression();
        compile_relational_operator(operator_token);
    }
}

static void compile_relational_operator(token_t operator_token) {
    if (DEBUG == true) printf("Compiling compile_relational_operator\n");

    switch (operator_token.type) {
        case TOKEN_EQ:
            return emit(OP_CMP_EQ, operator_token.line);
        case TOKEN_NE:
            return emit(OP_CMP_NE, operator_token.line);
        case TOKEN_GT:
            return emit(OP_CMP_GT, operator_token.line);
        case TOKEN_GE:
            return emit(OP_CMP_GE, operator_token.line);
        case TOKEN_LT:
            return emit(OP_CMP_LT, operator_token.line);
        case TOKEN_LE:
            return emit(OP_CMP_LE, operator_token.line);
        default:
            return error_throw(ERROR_COMPILER, "Unknown relational operator", operator_token.line);
    }
}

static void compile_additive_expression() {
    if (DEBUG == true) printf("Compiling compile_additive_expression\n");

    compile_multiplicative_expression();

    while (peek().type == TOKEN_PLUS || peek().type == TOKEN_MINUS) {
        token_t operator_token = advance();

        compile_multiplicative_expression();
        compile_binary_operator(operator_token);
    }
}

static void compile_multiplicative_expression() {
    if (DEBUG == true) printf("Compiling compile_multiplicative_expression\n");

    compile_access();

    while (peek().type == TOKEN_STAR || peek().type == TOKEN_SLASH) {
        token_t operator_token = advance();

        compile_access();
        compile_binary_operator(operator_token);
    }
}

// TODO: add member expressions
static void compile_access() {
    compile_call_expression();

    while (peek().type == TOKEN_OPEN_BRACKET) {
        int line = assert(TOKEN_OPEN_BRACKET).line;
        compile_expression();
        assert(TOKEN_CLOSE_BRACKET);
        emit(OP_ARRAY_GET, line);
    }
}

static void compile_binary_operator(token_t operator_token) {
    if (DEBUG == true) printf("Compiling compile_binary_operator\n");

    switch (operator_token.type) {
        case TOKEN_PLUS:
            return emit(OP_ADD, operator_token.line);
        case TOKEN_MINUS:
            return emit(OP_SUB, operator_token.line);
        case TOKEN_STAR:
            return emit(OP_MUL, operator_token.line);
        case TOKEN_SLASH:
            return emit(OP_DIV, operator_token.line);
        default:
            return error_throw(ERROR_COMPILER, "Unknown binary operator", operator_token.line);
    }
}

static void compile_call_expression() {
    if (DEBUG == true) printf("Compiling compile_call_expression\n");

    compile_primary_expression();

    if (peek().type == TOKEN_OPEN_PAREN) {
        int line = advance().line;
        double arg_count = compile_call_expression_args();
        assert(TOKEN_CLOSE_PAREN);

        emit_numeric_literal_num(arg_count, line);
        emit(OP_CALL, line);
    }
}

static double compile_call_expression_args() {
    if (DEBUG == true) printf("Compiling compile_call_expression_args\n");

    double arg_count = 0;

    while (peek().type != TOKEN_CLOSE_PAREN) {
        compile_expression();
        arg_count++;

        if (peek().type == TOKEN_COMMA) {
            advance();
        }
    }

    return arg_count;
}

static void compile_primary_expression() {
    if (DEBUG == true) printf("Compiling compile_primary_expression\n");

    switch (peek().type) {
        case TOKEN_OPEN_PAREN:
            return compile_parenthesised_expression();
        case TOKEN_IDENTIFIER:
            return compile_identifier();
        case TOKEN_NUMERIC_LITERAL:
            return compile_numeric_literal();
        case TOKEN_BOOLEAN_LITERAL:
            return compile_boolean_literal();
        case TOKEN_STRING_LITERAL:
            return compile_string_literal();
        default:
            return error_throw(ERROR_COMPILER, "Unknown primary expression", peek().line);
    }
}

static void compile_parenthesised_expression() {
    if (DEBUG == true) printf("Compiling compile_parenthesised_expression\n");

    assert(TOKEN_OPEN_PAREN);
    compile_expression();
    assert(TOKEN_CLOSE_PAREN);
}

static void compile_identifier() {
    if (DEBUG == true) printf("Compiling compile_identifier\n");

    token_t token = assert(TOKEN_IDENTIFIER);

    emit_string_literal(substring(token.start, token.length), token.line);
    emit(OP_LOAD_VAR, token.line);
}

static void compile_numeric_literal() {
    if (DEBUG == true) printf("Compiling compile_numeric_literal\n");

    token_t token = advance();
    emit_numeric_literal(substring(token.start, token.length), token.line);
}

static void compile_boolean_literal() {
    if (DEBUG == true) printf("Compiling compile_boolean_literal\n");

    token_t token = advance();
    emit_boolean_literal(substring(token.start, token.length), token.line);
}

static void compile_string_literal() {
    if (DEBUG == true) printf("Compiling compile_string_literal\n");

    token_t token = advance();
    emit_string_literal(substring(token.start, token.length), token.line);
}
