#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

#include "lexer/lexer.h"
#include "compiler/compiler.h"
#include "compiler/bytecode.h"
#include "compiler/instruction.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#include "utils/common.h"

const char* OP_CODE_LABELS[] = {
    "LOAD_NUM_CONST",
    "LOAD_BOOL_CONST",
    "LOAD_STR_CONST",

    "LOAD_VAR",
    "STORE_VAR",

    "FUNC_DEF",
    "FUNC_END",
    "RETURN",
    "CALL",

    "OBJ_DEF",
    "OBJ_END",
    "NEW_OBJ",
    "LOAD_PROP",
    "LOAD_PROP_CONST",
    "STORE_PROP",

    "ARRAY_DEF",
    "ARRAY_GET",
    "ARRAY_SET",

    "SIZE_OF",

    "LABEL",
    "JUMP",
    "JUMP_IF_FALSE",

    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "DIV_FLOOR",
    "NEG",

    "CMP_EQ",
    "CMP_NE",
    "CMP_LT",
    "CMP_LE",
    "CMP_GT",
    "CMP_GE",

    "AND",
    "OR",

    "PRINT",
    "NEWLINE",

    "STACK_CLEAR",
};

static void print_bytecode(bytecode_t* bytecode);

void interpreter_init(const char* source_code) {
    lexer_init(source_code);
    compiler_init();
}

void interpret() {
    bytecode_t* bytecode = compile();
    printf("INFO: Compiled\n");
    printf("------------------------------\n");

    print_bytecode(bytecode);

    vm_init(bytecode);

    clock_t start = clock();
    vm_run();
    clock_t end = clock();

    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("------------------------------\n");
    printf("INFO: Finished in %.2fs\n", elapsed_time);
}

static void print_bytecode(bytecode_t* bytecode) {
    //return;

    printf("BYTECODE --------------------\n");

    for (int i = 0; i < bytecode->count; ++i) {
        int op_index = i;

        switch (bytecode->instructions[i]) {
            case OP_LOAD_NUM_CONST: {
                printf("%d: %s\n", op_index, OP_CODE_LABELS[bytecode->instructions[i++]]);

                byte_t bytes[2];
                for (int j = i; j < i + 2; j++) {
                    bytes[j - i] = bytecode->instructions[j];
                }

                printf("%d: %d\n", op_index + 1, bytes[0]);
                printf("%d: %d\n", op_index + 2, bytes[1]);

                i += 1;
                break;
            }
            case OP_LOAD_STR_CONST: {
                printf("%d: %s\n", op_index, OP_CODE_LABELS[bytecode->instructions[i++]]);

                byte_t bytes[2];
                for (int j = i; j < i + 2; j++) {
                    bytes[j - i] = bytecode->instructions[j];
                }

                printf("%d: %d\n", op_index + 1, bytes[0]);
                printf("%d: %d\n", op_index + 2, bytes[1]);

                i += 1;
                break;
            }
            case OP_LOAD_BOOL_CONST: {
                printf("%d: %s\n", op_index, OP_CODE_LABELS[bytecode->instructions[i++]]);

                byte_t bool_value = bytecode->instructions[i];
                printf("%d: %s\n", op_index + 1, bool_value == 1 ? "true" : "false");

                break;
            }
            default: {
                printf("%d: %s\n", op_index, OP_CODE_LABELS[bytecode->instructions[i]]);
                break;
            }
        }
    }

    printf("-----------------------------\n");
}