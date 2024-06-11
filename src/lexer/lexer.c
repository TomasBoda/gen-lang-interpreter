#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer/lexer.h"
#include "utils/error.h"
#include "utils/common.h"

typedef struct {
    const char* start;
    const char* current;
    int line;
} lexer_t;

lexer_t lexer;

void lexer_init(const char* source_code) {
    lexer.start = source_code;
    lexer.current = source_code;
    lexer.line = 1;
}

static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_at_end() {
    return *lexer.current == '\0';
}

static char advance() {
    lexer.current++;
    return lexer.current[-1];
}

static char peek() {
    return *lexer.current;
}

static char peek_next() {
    if (is_at_end()) {
        return '\0';
    }

    return lexer.current[1];
}

static bool match(char expected) {
    if (is_at_end()) {
        return false;
    }

    if (*lexer.current != expected) {
        return false;
    }

    lexer.current++;
    return true;
}

static token_t make_token(token_type type) {
    token_t token;
    token.type = type;
    token.start = lexer.start;
    token.length = (int)(lexer.current - lexer.start);
    token.line = lexer.line;

    // trim the double quotes
    if (type == TOKEN_STRING_LITERAL) {
        token.start++;
        token.length -= 2;
    }

    //printf("token type = %d token value = %s\n", token.type, substring(token.start, token.length));

    return token;
}

static void skip_whitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                lexer.line++;
                advance();
                break;
            case '/':
                if (peek_next() == '/') {
                    while (peek() != '\n' && !is_at_end()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

static token_type check_keyword(int start, int length, const char* rest, token_type type) {
    if (lexer.current - lexer.start == start + length &&
        memcmp(lexer.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static token_type identifier_type() {
    switch (lexer.start[0]) {
        case 'a': return check_keyword(1, 2, "nd", TOKEN_AND);
        case 'b': return check_keyword(1, 4, "reak", TOKEN_BREAK);
        case 'c': return check_keyword(1, 7, "ontinue", TOKEN_CONTINUE);
        case 'e':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'l': return check_keyword(2, 2, "se", TOKEN_ELSE);
                    case 'n': return check_keyword(2, 2, "dl", TOKEN_ENDL);
                }
            }
            break;
        case 'f':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'u': return check_keyword(2, 2, "nc", TOKEN_FUNC);
                    case 'a': return check_keyword(2, 3, "lse", TOKEN_BOOLEAN_LITERAL);
                }
            }
            break;
        case 'i': return check_keyword(1, 1, "f", TOKEN_IF);
        case 'n': return check_keyword(1, 2, "ew", TOKEN_NEW);
        case 'o':
            if (lexer.current - lexer.start > 1) {
                switch (lexer.start[1]) {
                    case 'r': return check_keyword(2, 0, "", TOKEN_OR);
                    case 'b': return check_keyword(2, 4, "ject", TOKEN_OBJECT);
                }
            }
            break;
        case 'p': return check_keyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return check_keyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return check_keyword(1, 5, "izeof", TOKEN_SIZEOF);
        case 't': return check_keyword(1, 3, "rue", TOKEN_BOOLEAN_LITERAL);
        case 'v': return check_keyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return check_keyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;
}

static token_t identifier() {
    while (is_alpha(peek()) || is_digit(peek())) advance();
    return make_token(identifier_type());
}

static token_t number() {
    while (is_digit(peek())) advance();

    if (peek() == '.' && is_digit(peek_next())) {
        advance();

        while (is_digit(peek())) advance();
    }

    return make_token(TOKEN_NUMERIC_LITERAL);
}

static token_t string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') lexer.line++;
        advance();
    }

    if (is_at_end()) {
        error_throw(ERROR_COMPILER, "Unterminated string", lexer.line);
    };

    advance();
    return make_token(TOKEN_STRING_LITERAL);
}

token_t lexer_get_token() {
    skip_whitespace();

    lexer.start = lexer.current;

    if (is_at_end()) return make_token(TOKEN_EOF);

    char c = advance();

    if (is_alpha(c)) return identifier();
    if (is_digit(c)) return number();

    switch (c) {
        case '(': return make_token(TOKEN_OPEN_PAREN);
        case ')': return make_token(TOKEN_CLOSE_PAREN);
        case '{': return make_token(TOKEN_OPEN_BRACE);
        case '}': return make_token(TOKEN_CLOSE_BRACE);
        case '[': return make_token(TOKEN_OPEN_BRACKET);
        case ']': return make_token(TOKEN_CLOSE_BRACKET);
        case '.': return make_token(TOKEN_DOT);
        case ',': return make_token(TOKEN_COMMA);
        case ';': return make_token(TOKEN_SEMICOLON);
        case '+': return make_token(TOKEN_PLUS);
        case '-': return make_token(TOKEN_MINUS);
        case '*': return make_token(TOKEN_STAR);
        case '/': return make_token(TOKEN_SLASH);
        case '=': return make_token(match('=') ? TOKEN_EQ : TOKEN_ASSIGNMENT);
        case '!': return make_token(match('=') ? TOKEN_NE : TOKEN_EMPHASIS);
        case '<': return make_token(match('=') ? TOKEN_LE : TOKEN_LT);
        case '>': return make_token(match('=') ? TOKEN_GE : TOKEN_GT);
        case '"': return string();
    }

    error_throw(ERROR_COMPILER, "Unexpected character found during lexing", lexer.line);

    token_t* token = (token_t*)malloc(sizeof(token_t));
    return *token;
}
