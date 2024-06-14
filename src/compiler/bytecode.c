#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler/bytecode.h"
#include "utils/error.h"
#include "utils/common.h"

bytecode_t* bytecode_init() {
    bytecode_t* bytecode = (bytecode_t*)malloc(sizeof(bytecode_t));

    if (bytecode == NULL) {
        error_throw(ERROR_COMPILER, "Failed to allocate memory for bytecode", 0);
        return NULL;
    }

    bytecode->count = 0;
    bytecode->capacity = BYTECODE_INITIAL_SIZE;
    bytecode->instructions = (byte_t*)malloc(BYTECODE_INITIAL_SIZE * sizeof(byte_t));
    bytecode->lines = (int*)malloc(BYTECODE_INITIAL_SIZE * sizeof(int));

    if (bytecode->instructions == NULL) {
        free(bytecode);
        error_throw(ERROR_COMPILER, "Failed to allocate memory for bytecode instructions", 0);
        return NULL;
    }

    if (bytecode->lines == NULL) {
        free(bytecode);
        error_throw(ERROR_COMPILER, "Failed to allocate memory for bytecode lines", 0);
        return NULL;
    }

    return bytecode;
}

void bytecode_add(bytecode_t* bytecode, byte_t instruction, int line) {
    if (bytecode->count == bytecode->capacity) {
        bytecode->capacity += BYTECODE_INITIAL_SIZE;
        bytecode->instructions = (byte_t*)realloc(bytecode->instructions, bytecode->capacity * sizeof(byte_t));
        bytecode->lines = (int*)realloc(bytecode->lines, bytecode->capacity * sizeof(int));

        if (bytecode->instructions == NULL) {
            error_throw(ERROR_COMPILER, "Failed to reallocate memory for bytecode", 0);
            return;
        }

        if (bytecode->lines == NULL) {
            error_throw(ERROR_COMPILER, "Failed to reallocate memory for bytecode", 0);
            return;
        }
    }

    bytecode->instructions[bytecode->count] = instruction;
    bytecode->lines[bytecode->count] = line;
    bytecode->count++;
}
