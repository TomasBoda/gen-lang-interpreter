#ifndef gen_lang_compiler_h
#define gen_lang_compiler_h

#include <stdio.h>
#include <stdlib.h>

#include "compiler/bytecode.h"
#include "lexer/token.h"
#include "vm/pool.h"

/**
 * @brief Object representing a compiler holding the instruction array and a constant pool
 * 
 */
typedef struct {
    bytecode_t* bytecode;
    token_t current_token;
    pool_t* pool;
} compiler_t;

/**
 * @brief Initializes the compiler
 * 
 */
void compiler_init();

/**
 * @brief Compiles the source code into an array of instructions (bytecode)
 * 
 * @return bytecode_t* array of instructions (bytecode)
 */
bytecode_t* compile();

/**
 * @brief Retrieves the constant pool generated at compile time
 * 
 * @return pool_t* pointer to the constant pool object
 */
pool_t* compiler_get_pool();

#endif
