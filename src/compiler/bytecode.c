#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler/bytecode.h"
#include "utils/error.h"
#include "utils/common.h"

bytecode_t* bytecode_init() {
    bytecode_t* bytecode = (bytecode_t*)malloc(sizeof(bytecode_t));

    if (bytecode == NULL) {
        fprintf(stderr, "Failed to allocate memory for array.\n");
        exit(EXIT_FAILURE);
    }

    bytecode->count = 0;
    bytecode->capacity = BYTECODE_INITIAL_SIZE;
    bytecode->instructions = (byte_t*)malloc(BYTECODE_INITIAL_SIZE * sizeof(byte_t));

    if (bytecode->instructions == NULL) {
        free(bytecode);
        error_throw(error_type_compiler, "Failed to allocate memory for bytecode array", 0);
    }

    return bytecode;
}

void bytecode_add(bytecode_t* bytecode, byte_t instruction) {
    if (bytecode->count == bytecode->capacity) {
        bytecode->capacity += BYTECODE_INITIAL_SIZE;
        bytecode->instructions = (byte_t*)realloc(bytecode->instructions, bytecode->capacity * sizeof(byte_t));

        if (bytecode->instructions == NULL) {
            error_throw(error_type_compiler, "Failed to reallocate memory for bytecode array", 0);
        }
    }

    bytecode->instructions[bytecode->count] = instruction;
    bytecode->count++;
}
