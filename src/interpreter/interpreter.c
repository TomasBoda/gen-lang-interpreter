#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "lexer/lexer.h"
#include "compiler/compiler.h"
#include "compiler/bytecode.h"
#include "compiler/instruction.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#include "utils/common.h"

const char* op_code_labels[] = {
    "LOAD_CONST",
    "CONST_NUMERIC_LITERAL",
    "CONST_BOOLEAN_LITERAL",
    "CONST_STRING_LITERAL",

    "LOAD_GLOBAL",
    "STORE_GLOBAL",
    "LOAD_LOCAL",
    "STORE_LOCAL",

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
    "MOD",
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
};

static void print_bytecode(bytecode_t* bytecode);

void interpreter_init(const char* source_code) {
    lexer_init(source_code);
    compiler_init();
}

void interpret() {
    bytecode_t* bytecode = compile();

    printf("BYTECODE --------------------\n");
    print_bytecode(bytecode);
    printf("-----------------------------\n");

    printf("OUTPUT ----------------------\n");
    vm_run(bytecode);
    printf("-----------------------------\n");
}

static void print_bytecode(bytecode_t* bytecode) {
    for (int i = 0; i < bytecode->count; ++i) {
        switch (bytecode->instructions[i]) {
            case OP_CONST_NUMERIC_LITERAL: {
                printf("%s\n", op_code_labels[bytecode->instructions[i++]]);

                byte_t bytes[8];
                for (int j = i; j < i + 8; j++) {
                    bytes[j - i] = bytecode->instructions[j];
                }

                double value = bytes_to_double(bytes);

                if (value == (int)value) {
                    printf("%d\n", (int)value);
                } else {
                    printf("%.2f\n", value);
                }

                i += 7;
                break;
            }
            case OP_CONST_BOOLEAN_LITERAL: {
                printf("%s\n", op_code_labels[bytecode->instructions[i++]]);
                byte_t value = bytecode->instructions[i];
                printf(value == 1 ? "true\n" : "false\n");
                break;
            }
            case OP_CONST_STRING_LITERAL: {
                printf("%s\n", op_code_labels[bytecode->instructions[i++]]);
                byte_t size = bytecode->instructions[i++];
                byte_t* bytes = (byte_t*)malloc(size * sizeof(byte_t));

                for (int j = i; j < i + size + 1; j++) {
                    bytes[j - i] = bytecode->instructions[j];
                }

                printf("%s\n", bytes_to_string(bytes, size));
                i += size - 1;
                break;
            }
            default: {
                printf("%s\n", op_code_labels[bytecode->instructions[i]]);
                break;
            }
        }
    }
}