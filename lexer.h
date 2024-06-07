#ifndef gen_lang_lexer_h
#define gen_lang_lexer_h

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    TOKEN_VAR,
    TOKEN_FUNC,
    TOKEN_OBJECT,
    TOKEN_RETURN,
    TOKEN_NEW,
    TOKEN_SIZEOF,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_PRINT,

    TOKEN_IDENTIFIER,
    TOKEN_NUMERIC_LITERAL,
    TOKEN_BOOLEAN_LITERAL,
    TOKEN_STRING_LITERAL,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_MODULO,

    TOKEN_EMPHASIS,
    TOKEN_ASSIGNMENT,

    TOKEN_EQ,
    TOKEN_NE,
    TOKEN_GT,
    TOKEN_GE,
    TOKEN_LT,
    TOKEN_LE,

    TOKEN_OPEN_PAREN,
    TOKEN_CLOSE_PAREN,
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_OPEN_BRACKET,
    TOKEN_CLOSE_BRACKET,

    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,

    TOKEN_EOF,
} token_type;

typedef struct {
    token_type type;
    const char* start;
    int length;
    int line;
} token_t;

void lexer_init(const char* source_code);
token_t lexer_get_token();

#endif
