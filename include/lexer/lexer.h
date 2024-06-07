#ifndef gen_lang_lexer_h
#define gen_lang_lexer_h

#include "token.h"

void lexer_init(const char* source_code);
token_t lexer_get_token();

#endif
