#ifndef gen_lang_lexer_h
#define gen_lang_lexer_h

#include "token.h"

/**
 * @brief Initializes the lexer object
 * 
 * @param source_code source code to tokenize
 */
void lexer_init(const char* source_code);

/**
 * @brief Retrieves the next token from the source code
 * 
 * @return token_t generated token from the source code
 */
token_t lexer_get_token();

#endif
