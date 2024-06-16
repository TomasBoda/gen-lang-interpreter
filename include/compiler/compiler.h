#ifndef gen_lang_compiler_h
#define gen_lang_compiler_h

#include <stdio.h>
#include <stdlib.h>

#include "compiler/bytecode.h"
#include "compiler/stack.h"
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
    stack_long_t* continue_stack;
} compiler_t;

/**
 * @brief Initializes the compiler
 * 
 */
compiler_t* compiler_init(const char* source_code);

/**
 * @brief Compiles the source code into an array of instructions (bytecode)
 * 
 * @return bytecode_t* array of instructions (bytecode)
 */
bytecode_t* compile(compiler_t* compiler_instance);

/**
 * @brief Retrieves the constant pool generated at compile time
 * 
 * @return pool_t* pointer to the constant pool object
 */
pool_t* compiler_get_pool();

#endif
