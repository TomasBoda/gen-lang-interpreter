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

static void run_jump_if_false();
static void run_jump();
static void run_label();

static void run_add();
static void run_sub();
static void run_mul();
static void run_div();
static void run_cmp_eq();
static void run_cmp_ne();
static void run_cmp_gt();
static void run_cmp_ge();
static void run_cmp_lt();
static void run_cmp_le();

static void run_and();
static void run_or();

static void run_print();
static void run_print_numeric_literal(value_t* value);
static void run_print_boolean_literal(value_t* value);
static void run_print_string_literal(value_t* value);

static void run_stack_clear();

static void run_not_implemented() {
    error_throw(ERROR_RUNTIME, "Unrecognized instruction", 0);
}

typedef void (*instruction_handler)();

instruction_handler instruction_handlers[OP_NUM_INSTRUCTIONS] = {
    run_load_num_const,         // OP_LOAD_NUM_CONST
    run_load_bool_const,        // OP_LOAD_BOOL_CONST
    run_load_str_const,         // OP_LOAD_STR_CONST

    run_load_var,               // OP_LOAD_VAR
    run_store_var,              // OP_STORE_VAR

    run_func_def,               // OP_FUNC_DEF
    run_func_end,               // OP_FUNC_END
    run_return,                 // OP_RETURN
    run_call,                   // OP_CALL

    run_not_implemented,        // OP_OBJ_DEF
    run_not_implemented,        // OP_OBJ_END
    run_not_implemented,        // OP_NEW_OBJ
    run_not_implemented,        // OP_LOAD_PROP
    run_not_implemented,        // OP_LOAD_PROP_CONST
    run_not_implemented,        // OP_STORE_PROP

    run_not_implemented,        // OP_ARRAY_DEF
    run_not_implemented,        // OP_ARRAY_GET
    run_not_implemented,        // OP_ARRAY_SET

    run_not_implemented,        // OP_SIZE_OF

    run_label,                  // OP_LABEL
    run_jump,                   // OP_JUMP
    run_jump_if_false,          // OP_JUMP_IF_FALSE

    run_add,                    // OP_ADD
    run_sub,                    // OP_SUB
    run_mul,                    // OP_MUL
    run_div,                    // OP_DIV
    run_not_implemented,        // OP_DIV_FLOOR
    run_not_implemented,        // OP_NEG

    run_cmp_eq,                 // OP_CMP_EQ
    run_cmp_ne,                 // OP_CMP_NE
    run_cmp_lt,                 // OP_CMP_LT
    run_cmp_le,                 // OP_CMP_LE
    run_cmp_gt,                 // OP_CMP_GT
    run_cmp_ge,                 // OP_CMP_GE

    run_and,                    // OP_AND
    run_or,                     // OP_OR

    run_print,                  // OP_PRINT
    run_not_implemented,        // OP_NEWLINE

    run_stack_clear,            // OP_STACK_CLEAR
};

static inline byte_t current() {
    return vm.bytecode->instructions[vm.ip];
}

static inline byte_t next() {
    return vm.bytecode->instructions[vm.ip++];
}

static inline byte_t has_next() {
    return vm.ip < vm.bytecode->count;
}

static inline void stack_push(value_t value) {
    if (vm.stack_top == vm.stack + 256) {
        error_throw(ERROR_RUNTIME, "Stack overflow (max capacity = 256)", 0);
    }

    *vm.stack_top = value;
    vm.stack_top++;
}

static inline value_t stack_pop() {
    vm.stack_top--;
    return *vm.stack_top;
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

void vm_run() {
    while (has_next()) {
        if (current() >= 0 && current() < OP_NUM_INSTRUCTIONS) {
            instruction_handlers[next()]();
        } else {
            error_throw(ERROR_RUNTIME, "Instruction is unknown", 0);
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

    double numeric_literal = bytes_to_double(bytes);
    stack_push(value_create_number(numeric_literal));
}

static void run_load_bool_const() {
    if (DEBUG == true) printf("Running run_load_bool_const\n");

    byte_t boolean_literal = next();
    stack_push(value_create_boolean(boolean_literal == 1));
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

    stack_push(value_create_string(string_literal));
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

    return NULL;
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

    while (vm.ip < vm.bytecode->count) {
        if (current() == OP_LOAD_NUM_CONST) {
            next();
            vm.ip += 8;
            continue;
        }

        if (current() == OP_LOAD_BOOL_CONST) {
            next();
            vm.ip += 1;
            continue;
        }

        if (current() == OP_LOAD_STR_CONST) {
            next();
            byte_t size = next();
            vm.ip += size;
            continue;
        }

        if (current() == OP_FUNC_END) {
            next();
            break;
        }

        vm.ip += 1;
    }
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

    // exit the virtual machine
    if (call_stack_current(vm.call_stack) == NULL) {
        vm.ip = vm.bytecode->count;
    }
}

static inline long get_label_ip(long label_index) {
    long current_ip = 0;

    while (current_ip < vm.bytecode->count) {
        if (vm.bytecode->instructions[current_ip] == OP_LABEL) {
            if (vm.bytecode->instructions[++current_ip] == OP_LOAD_NUM_CONST) {
                byte_t bytes[8];
                for (int i = 0; i < 8; i++) {
                    bytes[i] = vm.bytecode->instructions[current_ip + 1 + i];
                }

                long found_label_index = (long) bytes_to_double(bytes);
                current_ip += 8;

                if (found_label_index == label_index) {
                    return current_ip + 1;
                }
            }
        } else {
            current_ip++;
        }
    }

    error_throw(ERROR_RUNTIME, "Could not find a label with the given label index", 0);
    return 0;
}

static void run_jump_if_false() {
    if (DEBUG == true) printf("Running run_jump_if_false\n");

    value_t label_index_value = stack_pop();
    value_t boolean_value = stack_pop();

    if (label_index_value.type != TYPE_NUMBER) {
        return error_throw(ERROR_RUNTIME, "Label index is not a number", 0);
    }

    if (boolean_value.type != TYPE_BOOLEAN) {
        return error_throw(ERROR_RUNTIME, "Jump if false operand is not a boolean", 0);
    }

    if (boolean_value.as.boolean == false) {
        long jump_ip = get_label_ip((long)label_index_value.as.number);
        vm.ip = jump_ip;
    }
}

static void run_jump() {
    if (DEBUG == true) printf("Running run_jump\n");

    value_t label_index_value = stack_pop();

    if (label_index_value.type != TYPE_NUMBER) {
        return error_throw(ERROR_RUNTIME, "Label index is not a number", 0);
    }

    long jump_ip = get_label_ip((long)label_index_value.as.number);
    vm.ip = jump_ip;
}

static void run_label() {
    if (DEBUG == true) printf("Running run_label\n");
    vm.ip += 9;
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
        stack_push(value_create_number(value2.as.number + value1.as.number));
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

        stack_push(value_create_string(new_string));
        return;
    }
}

static void run_sub() {
    if (DEBUG == true) printf("Running run_sub\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(value_create_number(value2.as.number - value1.as.number));
}

static void run_mul() {
    if (DEBUG == true) printf("Running run_mul\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(value_create_number(value2.as.number * value1.as.number));
}

static void run_div() {
    if (DEBUG == true) printf("Running run_div\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(value_create_number(value2.as.number / value1.as.number));
}

static void run_cmp_eq() {
    if (DEBUG == true) printf("Running run_cmp_eq\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_eq two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number == value1.as.number));
            break;
        }
        case TYPE_BOOLEAN: {
            stack_push(value_create_boolean(value2.as.boolean == value1.as.boolean));
            break;
        }
        case TYPE_STRING: {
            size_t strlen1 = strlen(value1.as.string);
            size_t strlen2 = strlen(value2.as.string);

            if (strlen1 != strlen2) {
                stack_push(value_create_boolean(false));
                break;
            }

            bool strings_equal = strcmp(value2.as.string, value1.as.string) == 0;
            stack_push(value_create_boolean(strings_equal));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_eq", 0);
        }
    }
}

static void run_cmp_ne() {
    if (DEBUG == true) printf("Running run_cmp_ne\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_ne two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number != value1.as.number));
            break;
        }
        case TYPE_BOOLEAN: {
            stack_push(value_create_boolean(value2.as.boolean != value1.as.boolean));
            break;
        }
        case TYPE_STRING: {
            size_t strlen1 = strlen(value1.as.string);
            size_t strlen2 = strlen(value2.as.string);

            if (strlen1 != strlen2) {
                stack_push(value_create_boolean(true));
                break;
            }

            bool strings_equal = strcmp(value2.as.string, value1.as.string) != 0;
            stack_push(value_create_boolean(strings_equal));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_ne", 0);
        }
    }
}

static void run_cmp_gt() {
    if (DEBUG == true) printf("Running run_cmp_gt\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_gt two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number > value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_gt", 0);
        }
    }
}

static void run_cmp_ge() {
    if (DEBUG == true) printf("Running run_cmp_ge\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_ge two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number >= value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_ge", 0);
        }
    }
}

static void run_cmp_lt() {
    if (DEBUG == true) printf("Running run_cmp_lt\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_lt two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number < value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_lt", 0);
        }
    }
}

static void run_cmp_le() {
    if (DEBUG == true) printf("Running run_cmp_le\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_le two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number <= value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_le", 0);
        }
    }
}

static void run_and() {
    if (DEBUG == true) printf("Running run_and\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot and two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_BOOLEAN: {
            stack_push(value_create_boolean(value2.as.boolean && value1.as.boolean));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for and", 0);
        }
    }
}

static void run_or() {
    if (DEBUG == true) printf("Running run_or\n");

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot or two values of different datatypes", 0);
        return;
    }

    switch (value1.type) {
        case TYPE_BOOLEAN: {
            stack_push(value_create_boolean(value2.as.boolean || value1.as.boolean));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for or", 0);
        }
    }
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

static void run_stack_clear() {
    if (DEBUG == true) printf("Running run_stack_clear\n");

    value_t stack_item_count = stack_pop();

    if (stack_item_count.type != TYPE_NUMBER) {
        return error_throw(ERROR_RUNTIME, "stack_item_count.type is not a number", 0);
    }

    for (int i = 0; i < (int)stack_item_count.as.number; i++) {
        stack_pop();
    }
}