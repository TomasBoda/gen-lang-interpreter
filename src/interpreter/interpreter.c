#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "lexer/lexer.h"
#include "compiler/compiler.h"
#include "compiler/bytecode.h"
#include "compiler/instruction.h"
#include "vm/vm.h"
#include "interpreter/interpreter.h"
#include "utils/common.h"

const char* OP_CODE_LABELS[] = {
    "LOAD_CONST",

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
    "ENDL",

    "STACK_CLEAR",
};

static void print_bytecode(bytecode_t* bytecode);

static const char* loaded_source_code;

char* get_line(const char* str, int i) {
    const char* start = str;
    const char* end = str;
    int line_num = 0;

    while (*end != '\0') {
        if (*end == '\n' || *(end + 1) == '\0') {
            if (line_num == i) {
                if (*(end + 1) == '\0' && *end != '\n') {
                    end++;
                }

                while (isspace((unsigned char)*start) && start < end) {
                    start++;
                }

                const char* line_end = end;
                while (line_end > start && isspace((unsigned char)*(line_end - 1))) {
                    line_end--;
                }

                size_t line_length = (size_t)(line_end - start);

                char* line = (char*)malloc(line_length + 1);
                if (!line) {
                    fprintf(stderr, "Memory allocation failed\n");
                    return NULL;
                }

                strncpy(line, start, line_length);
                line[line_length] = '\0';

                return line;
            }
            line_num++;
            start = end + 1;
        }
        end++;
    }
    return NULL;
}

void interpreter_init(const char* source_code) {
    lexer_init(source_code);
    compiler_init();

    loaded_source_code = source_code;
}

void interpret() {
    bytecode_t* bytecode = compile();
    printf("INFO: Compiled\n");
    printf("------------------------------\n");

    print_bytecode(bytecode);

    vm_init(bytecode);

    clock_t start = clock();
    vm_run(false);
    clock_t end = clock();

    double elapsed_time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("\n------------------------------\n");
    printf("INFO: Finished in %.2fs\n", elapsed_time);
}

static void print_bytecode(bytecode_t* bytecode) {
    return;

    printf("BYTECODE --------------------\n");

    int last_line = -1;

    for (int i = 0; i < bytecode->count - 7; ++i) {
        int op_index = i;

        if (bytecode->lines[i] != last_line) {
            printf("------------------------------\n");
            printf("%s\n", get_line(loaded_source_code, bytecode->lines[i] - 1));
            printf("------------------------------\n");
        }

        last_line = bytecode->lines[i];

        switch (bytecode->instructions[i]) {
            case OP_LOAD_CONST: {
                printf("%d: (line %d) %s\n", op_index, bytecode->lines[i], OP_CODE_LABELS[bytecode->instructions[i++]]);

                byte_t bytes[2];
                for (int j = i; j < i + 2; j++) {
                    bytes[j - i] = bytecode->instructions[j];
                }

                printf("%d: (line %d) %d\n", op_index + 1, bytecode->lines[i], bytes[0]);
                printf("%d: (line %d) %d\n", op_index + 2, bytecode->lines[i], bytes[1]);

                i += 1;
                break;
            }
            default: {
                printf("%d: (line %d) %s\n", op_index, bytecode->lines[i], OP_CODE_LABELS[bytecode->instructions[i]]);
                break;
            }
        }
    }

    printf("-----------------------------\n");
}