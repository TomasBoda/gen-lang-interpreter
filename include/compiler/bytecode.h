#ifndef gen_lang_bytecode_h
#define gen_lang_bytecode_h

#include <stdio.h>
#include <stdlib.h>

#include "utils/common.h"

#define BYTECODE_INITIAL_SIZE 10

typedef struct {
    int count;
    int capacity;
    byte_t* instructions;
    int* lines;
} bytecode_t;

bytecode_t* bytecode_init();
void bytecode_add(bytecode_t* bytecode, byte_t instruction, int line);

#endif
