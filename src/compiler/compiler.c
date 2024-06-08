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
#include "utils/table.h"
#include "utils/common.h"

typedef struct {
    bytecode_t* bytecode;
    token_t current_token;
} compiler_t;

compiler_t compiler;

static void emit(byte_t instruction) {
    bytecode_add(compiler.bytecode, instruction);
}

static void emit_numeric_literal(char* raw_value) {
    emit(OP_LOAD_NUM_CONST);

    double value = atof(raw_value);
    byte_t* bytes = double_to_bytes(value);

    for (int i = 0; i < 8; i++) {
        emit(bytes[i]);
    }
}

static void emit_numeric_literal_num(double value) {
    emit(OP_LOAD_NUM_CONST);

    byte_t* bytes = double_to_bytes(value);

    for (int i = 0; i < 8; i++) {
        emit(bytes[i]);
    }
}

static void emit_boolean_literal(char* raw_value) {
    emit(OP_LOAD_BOOL_CONST);

    if (strcmp(raw_value, "true") == 0) {
        emit(1);
    } else {
        emit(0);
    }
}

static void emit_string_literal(char* raw_value) {
    emit(OP_LOAD_STR_CONST);

    size_t instruction_count;
    byte_t* instructions = string_to_bytes(raw_value, &instruction_count);

    for (size_t i = 0; i < instruction_count; i++) {
        emit(instructions[i]);
    }
}

static double get_main_func_ip() {
    for (int i = 0; i < compiler.bytecode->count; i++) {
        if (compiler.bytecode->instructions[i] == OP_LOAD_STR_CONST) {
            byte_t size = compiler.bytecode->instructions[i + 1];
            byte_t bytes[size];
            for (int j = 0; j < size; j++) {
                bytes[j] = compiler.bytecode->instructions[i + 2 + j];
            }

            char* identifier = bytes_to_string(bytes, size);

            if (strcmp(identifier, "main") == 0 && compiler.bytecode->instructions[i + 1 + size + 1] == OP_FUNC_DEF) {
                return (double)(i + 1 + size + 1 + 1);
            }
        }
    }

    error_throw(ERROR_COMPILER, "main() function is missing", 0);
    return 0;
}

static void emit_main_func_call() {
    double main_func_ip = get_main_func_ip();
    emit_numeric_literal_num(main_func_ip);
    emit_numeric_literal_num(0);
    emit(OP_CALL);
}

static void compile_var_declaration();
static void compile_func_declaration();
static void compile_func_declaration_param_list();
static void compile_func_declaration_body();
static void compile_return();
static void compile_print();

static void compile_expression();
static void compile_logical_expression();
static void compile_relational_expression();
static void compile_additive_expression();
static void compile_multiplicative_expression();
static void compile_call_expression();
static size_t compile_call_expression_args();
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
            case TOKEN_PRINT:
                compile_print();
                break;
            default:
                error_throw(ERROR_COMPILER, "Unrecognized top level statement", peek().line);
        }
    }

    emit_main_func_call();
    return compiler.bytecode;
}

static void compile_var_declaration() {
    assert(TOKEN_VAR);
    token_t identifier_token = assert(TOKEN_IDENTIFIER);
    assert(TOKEN_ASSIGNMENT);

    compile_expression();
    assert(TOKEN_SEMICOLON);

    emit_string_literal(substring(identifier_token.start, identifier_token.length));
    emit(OP_STORE_VAR);
}

// TODO
static void compile_func_declaration() {
    assert(TOKEN_FUNC);
    token_t identifier_token = assert(TOKEN_IDENTIFIER);

    emit_string_literal(substring(identifier_token.start, identifier_token.length));
    emit(OP_FUNC_DEF);

    assert(TOKEN_OPEN_PAREN);
    compile_func_declaration_param_list();
    assert(TOKEN_CLOSE_PAREN);

    assert(TOKEN_OPEN_BRACE);
    compile_func_declaration_body();
    assert(TOKEN_CLOSE_BRACE);

    emit_numeric_literal("0");
    emit(OP_RETURN);

    emit(OP_FUNC_END);
}

static void compile_func_declaration_param_list() {
    while (peek().type != TOKEN_CLOSE_PAREN) {
        assert(TOKEN_VAR);
        token_t identifier_token = assert(TOKEN_IDENTIFIER);

        emit_string_literal(substring(identifier_token.start, identifier_token.length));
        emit(OP_STORE_VAR);

        if (peek().type == TOKEN_COMMA) {
            advance();
        }
    }
}

static void compile_func_declaration_body() {
    while (peek().type != TOKEN_CLOSE_BRACE) {
        switch (peek().type) {
            case TOKEN_VAR: {
                compile_var_declaration();
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
            default: {
                error_throw(ERROR_COMPILER, "Unknown statement in function body", 0);
                break;
            }
        }
    }
}

static void compile_return() {
    assert(TOKEN_RETURN);
    compile_expression();
    assert(TOKEN_SEMICOLON);
    emit(OP_RETURN);
}

static void compile_print() {
    assert(TOKEN_PRINT);

    compile_expression();
    assert(TOKEN_SEMICOLON);

    emit(OP_PRINT);
}

static void compile_expression() {
    compile_logical_expression();
}

static void compile_logical_expression() {
    compile_relational_expression();

    while (peek().type == TOKEN_AND || peek().type == TOKEN_OR) {
        token_t operator_token = advance();

        compile_relational_expression();
        compile_logical_operator(operator_token);
    }
}

static void compile_logical_operator(token_t operator_token) {
    switch (operator_token.type) {
        case TOKEN_AND:
            return emit(OP_AND);
        case TOKEN_OR:
            return emit(OP_OR);
        default:
            return error_throw(ERROR_COMPILER, "Unknown logical operator", operator_token.line);
    }
}

static void compile_relational_expression() {
    compile_additive_expression();

    if (peek().type == TOKEN_EQ || peek().type == TOKEN_NE || peek().type == TOKEN_GT || peek().type == TOKEN_GE || peek().type == TOKEN_LT || peek().type == TOKEN_LE) {
        token_t operator_token = advance();

        compile_additive_expression();
        compile_relational_operator(operator_token);
    }
}

static void compile_relational_operator(token_t operator_token) {
    switch (operator_token.type) {
        case TOKEN_EQ:
            return emit(OP_CMP_EQ);
        case TOKEN_NE:
            return emit(OP_CMP_NE);
        case TOKEN_GT:
            return emit(OP_CMP_GT);
        case TOKEN_GE:
            return emit(OP_CMP_GE);
        case TOKEN_LT:
            return emit(OP_CMP_LT);
        case TOKEN_LE:
            return emit(OP_CMP_LE);
        default:
            return error_throw(ERROR_COMPILER, "Unknown relational operator", operator_token.line);
    }
}

static void compile_additive_expression() {
    compile_multiplicative_expression();

    while (peek().type == TOKEN_PLUS || peek().type == TOKEN_MINUS) {
        token_t operator_token = advance();

        compile_multiplicative_expression();
        compile_binary_operator(operator_token);
    }
}

static void compile_multiplicative_expression() {
    compile_call_expression();

    while (peek().type == TOKEN_STAR || peek().type == TOKEN_SLASH) {
        token_t operator_token = advance();

        compile_call_expression();
        compile_binary_operator(operator_token);
    }
}

static void compile_binary_operator(token_t operator_token) {
    switch (operator_token.type) {
        case TOKEN_PLUS:
            return emit(OP_ADD);
        case TOKEN_MINUS:
            return emit(OP_SUB);
        case TOKEN_STAR:
            return emit(OP_MUL);
        case TOKEN_SLASH:
            return emit(OP_DIV);
        default:
            return error_throw(ERROR_COMPILER, "Unknown binary operator", operator_token.line);
    }
}

static void compile_call_expression() {
    compile_primary_expression();

    if (peek().type == TOKEN_OPEN_PAREN) {
        advance();
        size_t arg_count = compile_call_expression_args();
        assert(TOKEN_CLOSE_PAREN);

        emit_numeric_literal_num((double)arg_count);
        emit(OP_CALL);

        // TODO: fix LOAD_LOCAL in identifier of call expression
    }
}

static size_t compile_call_expression_args() {
    size_t arg_count = 0;

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
    assert(TOKEN_OPEN_PAREN);
    compile_expression();
    assert(TOKEN_CLOSE_PAREN);
}

static void compile_identifier() {
    token_t token = assert(TOKEN_IDENTIFIER);

    emit_string_literal(substring(token.start, token.length));
    emit(OP_LOAD_VAR);
}

static void compile_numeric_literal() {
    token_t token = advance();
    emit_numeric_literal(substring(token.start, token.length));
}

static void compile_boolean_literal() {
    token_t token = advance();
    emit_boolean_literal(substring(token.start, token.length));
}

static void compile_string_literal() {
    token_t token = advance();
    emit_string_literal(substring(token.start, token.length));
}
