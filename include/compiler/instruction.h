#ifndef gen_lang_instruction_h
#define gen_lang_instruction_h

/**
 * @brief Instruction types
 * 
 */
typedef enum {
    OP_LOAD_CONST,

    OP_LOAD_VAR,
    OP_STORE_VAR,

    OP_FUNC_DEF,
    OP_FUNC_END,
    OP_RETURN,
    OP_CALL,

    OP_ENUM_DEF,
    OP_STORE_ENUM,
    OP_ENUM_END,

    OP_OBJ_DEF,
    OP_OBJ_END,
    OP_NEW_OBJ,
    OP_LOAD_PROP,
    OP_LOAD_PROP_CONST,
    OP_STORE_PROP,
    OP_INIT_PROP,

    OP_ARRAY_DEF,
    OP_ARRAY_GET,
    OP_ARRAY_SET,

    OP_SIZEOF,

    OP_JUMP,
    OP_JUMP_IF_FALSE,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_DIV_FLOOR,
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
    OP_ENDL,

    OP_STACK_CLEAR,

    OP_NUM_INSTRUCTIONS,
} op_code_t;

#endif