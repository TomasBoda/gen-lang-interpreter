#ifndef gen_lang_compiler_h
#define gen_lang_compiler_h

#include <stdio.h>
#include <stdlib.h>

#include "compiler/bytecode.h"
#include "lexer/token.h"
#include "vm/pool.h"

typedef struct {
    bytecode_t* bytecode;
    token_t current_token;
    pool_t* pool;
} compiler_t;

void compiler_init();
bytecode_t* compile();
pool_t* compiler_get_pool();

#endif
