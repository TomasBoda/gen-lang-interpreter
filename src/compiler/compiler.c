#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "../bytecode.h"
#include "compiler.h"
#include "../lexer/lexer.h"
#include "../error.h"
#include "../table.h"
#include "../common.h"

typedef struct {
    bytecode_t* bytecode;
    table_t* table;
    token_t current_token;
} compiler_t;

compiler_t compiler;

static value_t* load_global_variable(char* identifier) {
    return table_get(compiler.table, identifier);
}

static void store_global_variable(char* identifier) {
    value_t* value = (value_t*)malloc(sizeof(value_t));
    table_set(compiler.table, identifier, *value);
}

static void emit(byte_t instruction) {
    bytecode_add(compiler.bytecode, instruction);
}

static void emit_numeric_literal(token_t token) {
    emit(OP_LOAD_CONST);
    emit(OP_CONST_NUMERIC_LITERAL);

    double value = atof(substring(token.start, token.length));
    byte_t* bytes = double_to_bytes(value);

    for (int i = 0; i < 8; i++) {
        emit(bytes[i]);
    }
}

static void emit_boolean_literal(token_t token) {
    emit(OP_LOAD_CONST);
    emit(OP_CONST_BOOLEAN_LITERAL);

    if (strcmp(substring(token.start, token.length), "true") == 0) {
        emit(1);
    } else {
        emit(0);
    }
}

static void emit_string_literal(token_t token) {
    emit(OP_LOAD_CONST);
    emit(OP_CONST_STRING_LITERAL);

    size_t instruction_count;
    byte_t* instructions = string_to_bytes(substring(token.start, token.length), &instruction_count);

    for (size_t i = 0; i < instruction_count; i++) {
        emit(instructions[i]);
    }
}

static void compile_var_declaration();
static void compile_print();

static void compile_expression();
static void compile_logical_expression();
static void compile_relational_expression();
static void compile_additive_expression();
static void compile_multiplicative_expression();
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
        error_throw(error_type_compiler, "token assertion failed", token.line);
    }

    return token;
}

void compiler_init() {
    compiler.bytecode = bytecode_init();
    compiler.table = table_init(50);
    compiler.current_token = lexer_get_token();
}

bytecode_t* compile() {
    while (peek().type != TOKEN_EOF) {
        switch (peek().type) {
            case TOKEN_VAR:
                compile_var_declaration();
                break;
            case TOKEN_PRINT:
                compile_print();
                break;
            default:
                error_throw(error_type_compiler, "Unrecognized top level statement", peek().line);
        }
    }

    return compiler.bytecode;
}

static void compile_var_declaration() {
    assert(TOKEN_VAR);
    token_t identifier_token = assert(TOKEN_IDENTIFIER);
    assert(TOKEN_ASSIGNMENT);

    compile_expression();
    assert(TOKEN_SEMICOLON);

    emit_string_literal(identifier_token);
    emit(OP_STORE_GLOBAL);

    store_global_variable(substring(identifier_token.start, identifier_token.length));
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
            return error_throw(error_type_compiler, "Unknown logical operator", operator_token.line);
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
            return error_throw(error_type_compiler, "Unknown relational operator", operator_token.line);
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
    compile_primary_expression();

    while (peek().type == TOKEN_STAR || peek().type == TOKEN_SLASH) {
        token_t operator_token = advance();

        compile_primary_expression();
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
        case TOKEN_MODULO:
            return emit(OP_MOD);
        default:
            return error_throw(error_type_compiler, "Unknown binary operator", operator_token.line);
    }
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
            return error_throw(error_type_compiler, "Unknown primary expression", peek().line);
    }
}

static void compile_parenthesised_expression() {
    assert(TOKEN_OPEN_PAREN);
    compile_expression();
    assert(TOKEN_CLOSE_PAREN);
}

static void compile_identifier() {
    token_t token = advance();

    emit_string_literal(token);

    if (load_global_variable(substring(token.start, token.length)) == NULL) {
        error_throw(error_type_compiler, "Global variable does not exists", token.line);
    }

    emit(OP_LOAD_GLOBAL);
}

static void compile_numeric_literal() {
    emit_numeric_literal(advance());
}

static void compile_boolean_literal() {
    emit_boolean_literal(advance());
}

static void compile_string_literal() {
    emit_string_literal(advance());
}
