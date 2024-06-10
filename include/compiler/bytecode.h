#ifndef gen_lang_bytecode_h
#define gen_lang_bytecode_h

#include <stdio.h>
#include <stdlib.h>

#include "utils/common.h"

#define BYTECODE_INITIAL_SIZE 10

/**
 * @brief object representing the array of instructions (bytecode) and their corresponding line numbers
 * 
 */
typedef struct {
    int count;
    int capacity;
    byte_t* instructions;
    int* lines;
} bytecode_t;

/**
 * @brief Initializes the bytecode_t object representing the array of instructions and their line numbers
 * 
 * @return bytecode_t* initialized bytecode_t object pointer
 */
bytecode_t* bytecode_init();

/**
 * @brief Adds a new instruction to the bytecode array
 * 
 * @param bytecode bytecode_t object to store the bytecode to
 * @param instruction instruction byte to add
 * @param line line number of the instruction
 */
void bytecode_add(bytecode_t* bytecode, byte_t instruction, int line);

#endif
