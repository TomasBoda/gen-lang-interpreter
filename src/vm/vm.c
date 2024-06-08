#include <stdbool.h>
#include <string.h>

#include "compiler/bytecode.h"
#include "compiler/compiler.h"
#include "compiler/instruction.h"
#include "utils/table.h"
#include "utils/common.h"
#include "utils/error.h"
#include "vm/callstack.h"

bool DEBUG = false;

typedef struct {
    long ip;
    value_t stack[256];
    value_t* stack_top;
    bytecode_t* bytecode;
    table_t* table;
    call_stack_t* call_stack;
} virtual_machine_t;

virtual_machine_t vm;

static void run_load_num_const();
static void run_load_bool_const();
static void run_load_str_const();

static void run_load_var();
static void run_store_var();

static void run_func_def();
static void run_func_end();
static void run_call();
static void run_return();

static void run_add();
static void run_sub();
static void run_mul();
static void run_div();

static void run_print();
static void run_print_numeric_literal(value_t* value);
static void run_print_boolean_literal(value_t* value);
static void run_print_string_literal(value_t* value);

static byte_t current() {
    return vm.bytecode->instructions[vm.ip];
}

static byte_t next() {
    return vm.bytecode->instructions[vm.ip++];
}

static byte_t has_next() {
    return vm.ip < vm.bytecode->count;
}

void stack_push(value_t value) {
    *vm.stack_top = value;
    vm.stack_top++;
}

value_t stack_pop() {
    vm.stack_top--;
    return *vm.stack_top;
}

static value_t create_numeric_literal(double numeric_literal) {
    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = numeric_literal;
    return value;
}

static value_t create_boolean_literal(bool boolean_literal) {
    value_t value;
    value.type = TYPE_BOOLEAN;
    value.as.boolean = boolean_literal;
    return value;
}

static value_t create_string_literal(char* string_literal) {
    value_t value;
    value.type = TYPE_STRING;
    value.as.string = string_literal;
    return value;
}

void vm_init(bytecode_t* bytecode) {
    vm.bytecode = bytecode;
    vm.ip = 0;
    vm.stack_top = vm.stack;

    vm.table = table_init(50);
    vm.call_stack = call_stack_init();
}

static void vm_free() {
    if (vm.table != NULL) {
        table_free(vm.table);
        free(vm.table);
        vm.table = NULL;
    }

    if (vm.call_stack != NULL) {
        call_stack_free(vm.call_stack);
        free(vm.call_stack);
        vm.call_stack = NULL;
    }

    if (vm.bytecode != NULL) {
        vm.bytecode = NULL;
    }

    for (value_t* value = vm.stack; value < vm.stack_top; value++) {
        if (value->type == TYPE_STRING) {
            free(value->as.string);
        }
    }
}

const char* op_code_labels[] = {
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
};

void vm_run() {
    while (has_next()) {
        switch (next()) {
            case OP_LOAD_NUM_CONST: {
                run_load_num_const();
                break;
            }
            case OP_LOAD_BOOL_CONST: {
                run_load_bool_const();
                break;
            }
            case OP_LOAD_STR_CONST: {
                run_load_str_const();
                break;
            }
            case OP_LOAD_VAR: {
                run_load_var();
                break;
            }
            case OP_STORE_VAR: {
                run_store_var();
                break;
            }
            case OP_FUNC_DEF: {
                run_func_def();
                break;
            }
            case OP_FUNC_END: {
                run_func_end();
                break;
            }
            case OP_CALL: {
                run_call();
                break;
            }
            case OP_RETURN: {
                run_return();
                break;
            }
            case OP_ADD: {
                run_add();
                break;
            }
            case OP_SUB: {
                run_sub();
                break;
            }
            case OP_MUL: {
                run_mul();
                break;
            }
            case OP_DIV: {
                run_div();
                break;
            }
            case OP_PRINT: {
                run_print();
                break;
            }
            default: {
                error_throw(ERROR_RUNTIME, "Unrecognized instruction", 0);
                break;
            }
        }
    }

    vm_free();
}

static void run_load_num_const() {
    if (DEBUG == true) printf("Running run_load_num_const\n");

    byte_t bytes[8];

    for (size_t i = 0; i < 8; i++) {
        bytes[i] = next();
    }

    //vm.ip -= 1;

    double numeric_literal = bytes_to_double(bytes);
    stack_push(create_numeric_literal(numeric_literal));
}

static void run_load_bool_const() {
    if (DEBUG == true) printf("Running run_load_bool_const on ip = %d, op_type = %d\n", vm.ip);

    byte_t boolean_literal = next();
    stack_push(create_boolean_literal(boolean_literal == 1));
}

static void run_load_str_const() {
    if (DEBUG == true) printf("Running run_load_str_const\n");

    byte_t string_size = next();
    char* string_literal = (char*)malloc(sizeof(char) * string_size);

    if (string_literal == NULL) {
        error_throw(ERROR_RUNTIME, "Failed to allocate memory for string literal", 0);
        return;
    }

    for (int i = 0; i < string_size; i++) {
        string_literal[i] = (char)next();
    }

    stack_push(create_string_literal(string_literal));
}

static value_t* load_global_var(char* identifier) {
    return table_get(vm.table, identifier);
}

static value_t* load_local_var(char* identifier) {
    if (call_stack_current(vm.call_stack) == NULL) {
        return NULL;
    }

    return table_get(call_stack_current(vm.call_stack)->table, identifier);
}

static value_t* load_var(char* identifier) {
    value_t* global_var = load_global_var(identifier);
    value_t* local_var = load_local_var(identifier);

    if (global_var == NULL && local_var == NULL) {
        error_throw(ERROR_RUNTIME, "Variable with the given identifier does not exist", 0);
    }

    if (global_var != NULL) {
        return global_var;
    }

    if (local_var != NULL) {
        return local_var;
    }
}

static void run_load_var() {
    if (DEBUG == true) printf("Running run_load_var\n");

    value_t identifier = stack_pop();
    value_t* value = load_var(identifier.as.string);

    if (value == NULL) {
        return error_throw(ERROR_RUNTIME, "Variable with the given identifier does not exist", 0);
    }

    stack_push(*value);
}

static void run_store_var() {
    if (DEBUG == true) printf("Running run_store_var\n");

    value_t identifier = stack_pop();

    if (identifier.type != TYPE_STRING) {
        return error_throw(ERROR_RUNTIME, "in run_store_var(): value popped from stack is not a string (identifier)", 0);
    }

    value_t value = stack_pop();

    if (call_stack_current(vm.call_stack) == NULL) {
        table_set(vm.table, identifier.as.string, value);
    } else {
        table_set(call_stack_current(vm.call_stack)->table, identifier.as.string, value);
    }
}

static void run_func_def() {
    if (DEBUG == true) printf("Running run_func_def\n");

    value_t func_identifier = stack_pop();
    table_set(vm.table, func_identifier.as.string, value_create_number(vm.ip));

    while (current() != OP_FUNC_END) {
        next();
    }

    next();
}

static void run_func_end() {
    if (DEBUG == true) printf("Running run_func_end\n");
}

static void run_call() {
    if (DEBUG == true) printf("Running run_call\n");

    value_t func_arg_count = stack_pop();
    byte_t arg_count = func_arg_count.as.number;

    value_t args[arg_count];
    for (int i = 0; i < arg_count; i++) {
        args[i] = stack_pop();
    }

    value_t func_ip = stack_pop();

    call_frame_t call_frame;
    call_frame.ra = vm.ip;
    call_frame.table = table_init(50);

    call_stack_push(vm.call_stack, call_frame);

    vm.ip = (long)func_ip.as.number;

    for (int i = 0; i < arg_count; i++) {
        stack_push(args[i]);
    }
}

static void run_return() {
    if (DEBUG == true) printf("Running run_return\n");
    
    value_t return_value = stack_pop();
    call_frame_t call_frame = call_stack_pop(vm.call_stack);

    vm.ip = call_frame.ra;
    stack_push(return_value);

    call_frame_free(&call_frame);

    if (call_stack_current(vm.call_stack) == NULL) {
        printf("Finished running\n");
        exit(0);
    }
}

static void run_add() {
    if (DEBUG == true) printf("Running run_add\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot add two values of different datatypes", 0);
        return;
    }

    if (value2.type == TYPE_NUMBER && value1.type == TYPE_NUMBER) {
        stack_push(create_numeric_literal(value2.as.number + value1.as.number));
        return;
    }

    if (value2.type == TYPE_STRING && value1.type == TYPE_STRING) {
        size_t strlen1 = strlen(value1.as.string);
        size_t strlen2 = strlen(value2.as.string);

        char* new_string = (char*)malloc((strlen1 + strlen2) * sizeof(char));

        if (new_string == NULL) {
            error_throw(ERROR_RUNTIME, "Failed to allocate memory for string literal", 0);
            return;
        }

        for (int i = 0; i < strlen2; i++) {
            new_string[i] = value2.as.string[i];
        }

        for (int i = 0; i < strlen1; i++) {
            new_string[strlen2 + i] = value1.as.string[i];
        }

        stack_push(create_string_literal(new_string));
        return;
    }
}

static void run_sub() {
    if (DEBUG == true) printf("Running run_sub\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(create_numeric_literal(value2.as.number - value1.as.number));
}

static void run_mul() {
    if (DEBUG == true) printf("Running run_mul\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(create_numeric_literal(value2.as.number * value1.as.number));
}

static void run_div() {
    if (DEBUG == true) printf("Running run_div\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(create_numeric_literal(value2.as.number / value1.as.number));
}

static void run_print() {
    if (DEBUG == true) printf("Running run_print\n");

    value_t value = stack_pop();

    switch (value.type) {
        case TYPE_NUMBER: {
            return run_print_numeric_literal(&value);
        }
        case TYPE_BOOLEAN: {
            return run_print_boolean_literal(&value);
        }
        case TYPE_STRING: {
            return run_print_string_literal(&value);
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype in print statement", 0);
        }
    }
}

static void run_print_numeric_literal(value_t* value) {
    if (DEBUG == true) printf("Running run_print_numeric_literal\n");

    if (value->as.number == (int)value->as.number) {
        printf("%d\n", (int)value->as.number);
    } else {
        printf("%.2f\n", value->as.number);
    }
}

static void run_print_boolean_literal(value_t* value) {
    if (DEBUG == true) printf("Running run_print_boolean_literal\n");

    printf("%s\n", value->as.boolean ? "true" : "false");
}

static void run_print_string_literal(value_t* value) {
    if (DEBUG == true) printf("Running run_print_string_literal\n");

    printf("%s\n", value->as.string);
}