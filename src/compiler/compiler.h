#ifndef gen_lang_compiler_h
#define gen_lang_compiler_h

#include <stdio.h>
#include <stdlib.h>

#include "../bytecode.h"

typedef enum {
    OP_LOAD_CONST,
    OP_CONST_NUMERIC_LITERAL,
    OP_CONST_BOOLEAN_LITERAL,
    OP_CONST_STRING_LITERAL,

    OP_LOAD_GLOBAL,
    OP_STORE_GLOBAL,
    OP_LOAD_LOCAL,
    OP_STORE_LOCAL,

    OP_FUNC_DEF,
    OP_FUNC_END,
    OP_RETURN,
    OP_CALL,

    OP_OBJ_DEF,
    OP_OBJ_END,
    OP_NEW_OBJ,
    OP_LOAD_PROP,
    OP_LOAD_PROP_CONST,
    OP_STORE_PROP,

    OP_ARRAY_DEF,
    OP_ARRAY_GET,
    OP_ARRAY_SET,

    OP_SIZE_OF,

    OP_LABEL,
    OP_JUMP,
    OP_JUMP_IF_FALSE,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_DIV_FLOOR,
    OP_MOD,
    OP_NEG,

    OP_CMP_EQ,
    OP_CMP_NE,
    OP_CMP_LT,
    OP_CMP_LE,
    OP_CMP_GT,
    OP_CMP_GE,

    OP_AND,
    OP_OR,

    OP_PRINT,
    OP_NEWLINE,
} op_code_t;

void compiler_init();
bytecode_t* compile();

#endif