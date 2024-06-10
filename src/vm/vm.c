#include <stdbool.h>
#include <string.h>

#include "compiler/bytecode.h"
#include "compiler/compiler.h"
#include "compiler/instruction.h"
#include "utils/table.h"
#include "utils/common.h"
#include "utils/error.h"
#include "vm/callstack.h"
#include "vm/vm.h"
#include "vm/pool.h"

virtual_machine_t vm;

static inline int line() {
    return vm.bytecode->lines[vm.ip];
}

static inline byte_t current() {
    return vm.bytecode->instructions[vm.ip];
}

static inline byte_t next() {
    return vm.bytecode->instructions[vm.ip++];
}

static inline byte_t has_next() {
    return vm.ip < vm.bytecode->count;
}

static void run_load_const();

static void run_load_var();
static void run_store_var();

static void run_func_def();
static void run_func_end();
static void run_call();
static void run_return();

static void run_jump_if_false();
static void run_jump();

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
    error_throw(ERROR_RUNTIME, "Unrecognized instruction", line());
}

static inline void stack_push(value_t value) {
    if (vm.stack_top == vm.stack + 256) {
        error_throw(ERROR_RUNTIME, "Stack overflow (max capacity = 256)", line());
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
    vm.pool = compiler_get_pool();
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
    static void* dispatch_table[] = {
        &&label_load_const,             // OP_LOAD_CONST

        &&label_load_var,               // OP_LOAD_VAR
        &&label_store_var,              // OP_STORE_VAR

        &&label_func_def,               // OP_FUNC_DEF
        &&label_func_end,               // OP_FUNC_END
        &&label_return,                 // OP_RETURN
        &&label_call,                   // OP_CALL

        &&label_not_implemented,        // OP_OBJ_DEF
        &&label_not_implemented,        // OP_OBJ_END
        &&label_not_implemented,        // OP_NEW_OBJ
        &&label_not_implemented,        // OP_LOAD_PROP
        &&label_not_implemented,        // OP_LOAD_PROP_CONST
        &&label_not_implemented,        // OP_STORE_PROP

        &&label_not_implemented,        // OP_ARRAY_DEF
        &&label_not_implemented,        // OP_ARRAY_GET
        &&label_not_implemented,        // OP_ARRAY_SET

        &&label_not_implemented,        // OP_SIZE_OF

        &&label_jump,                   // OP_JUMP
        &&label_jump_if_false,          // OP_JUMP_IF_FALSE

        &&label_add,                    // OP_ADD
        &&label_sub,                    // OP_SUB
        &&label_mul,                    // OP_MUL
        &&label_div,                    // OP_DIV
        &&label_not_implemented,        // OP_DIV_FLOOR
        &&label_not_implemented,        // OP_NEG

        &&label_cmp_eq,                 // OP_CMP_EQ
        &&label_cmp_ne,                 // OP_CMP_NE
        &&label_cmp_lt,                 // OP_CMP_LT
        &&label_cmp_le,                 // OP_CMP_LE
        &&label_cmp_gt,                 // OP_CMP_GT
        &&label_cmp_ge,                 // OP_CMP_GE

        &&label_and,                    // OP_AND
        &&label_or,                     // OP_OR

        &&label_print,                  // OP_PRINT
        &&label_not_implemented,        // OP_NEWLINE

        &&label_stack_clear,            // OP_STACK_CLEAR
    };

    #define DISPATCH() goto *dispatch_table[next()];

    while (has_next()) {
        DISPATCH();

        label_load_const:
            //printf("run_load_const\n");
            run_load_const();
            DISPATCH();

        label_load_var:
            //printf("run_load_var\n");
            run_load_var();
            DISPATCH();

        label_store_var:
            //printf("run_store_var\n");
            run_store_var();
            DISPATCH();

        label_func_def:
            //printf("run_func_def\n");
            run_func_def();
            DISPATCH();

        label_func_end:
            //printf("run_func_end\n");
            run_func_end();
            DISPATCH();

        label_return:
            //printf("run_return\n");
            run_return();
            if (!has_next()) break;
            DISPATCH();

        label_call:
            //printf("run_call\n");
            run_call();
            DISPATCH();

        label_not_implemented:
            //printf("run_not_implemented\n");
            run_not_implemented();
            DISPATCH();

        label_jump:
            //printf("run_jump\n");
            run_jump();
            DISPATCH();

        label_jump_if_false:
            //printf("run_jump_if_false\n");
            run_jump_if_false();
            DISPATCH();

        label_add:
            //printf("run_add\n");
            run_add();
            DISPATCH();

        label_sub:
            //printf("run_sub\n");
            run_sub();
            DISPATCH();

        label_mul:
            //printf("run_mul\n");
            run_mul();
            DISPATCH();

        label_div:
            //printf("run_div\n");
            run_div();
            DISPATCH();

        label_cmp_eq:
            //printf("run_cmp_eq\n");
            run_cmp_eq();
            DISPATCH();

        label_cmp_ne:
            //printf("run_cmp_ne\n");
            run_cmp_ne();
            DISPATCH();

        label_cmp_lt:
            //printf("run_cmp_lt\n");
            run_cmp_lt();
            DISPATCH();

        label_cmp_le:
            //printf("run_cmp_le\n");
            run_cmp_le();
            DISPATCH();

        label_cmp_gt:
            //printf("run_cmp_gt\n");
            run_cmp_gt();
            DISPATCH();

        label_cmp_ge:
            //printf("run_cmp_ge\n");
            run_cmp_ge();
            DISPATCH();

        label_and:
            //printf("run_and\n");
            run_and();
            DISPATCH();

        label_or:
            //printf("run_or\n");
            run_or();
            DISPATCH();

        label_print:
            //printf("run_print\n");
            run_print();
            DISPATCH();

        label_stack_clear:
            //printf("run_stack_clear\n");
            run_stack_clear();
            DISPATCH();
    }

    vm_free();
}

static void run_load_const() {
    byte_t bytes[2];
    bytes[0] = next();
    bytes[1] = next();

    uint16_t pool_index = bytes_to_uint16(bytes);
    value_t* value = pool_get(vm.pool, pool_index);

    stack_push(*value);
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
        error_throw(ERROR_RUNTIME, "Variable with the given identifier does not exist", line());
    }

    return global_var == NULL ? local_var : global_var;
}

static void run_load_var() {
    value_t identifier = stack_pop();
    value_t* value = load_var(identifier.as.string);

    stack_push(*value);
}

static void run_store_var() {
    value_t identifier = stack_pop();
    value_t value = stack_pop();

    if (call_stack_current(vm.call_stack) == NULL) {
        table_set(vm.table, identifier.as.string, value);
    } else {
        table_set(call_stack_current(vm.call_stack)->table, identifier.as.string, value);
    }
}

static inline void skip_func_def() {
    while (vm.ip < vm.bytecode->count) {
        switch (current()) {
            case OP_LOAD_CONST: {
                next();
                vm.ip += 2;
                break;
            }
            case OP_FUNC_END: {
                next();
                return;
            }
            default: {
                vm.ip += 1;
                break;
            }
        }
    }
}

static void run_func_def() {
    value_t func_identifier = stack_pop();
    table_set(vm.table, func_identifier.as.string, value_create_number(vm.ip));

    skip_func_def();
}

static void run_func_end() {}

static void run_call() {
    value_t func_arg_count = stack_pop();
    byte_t arg_count = func_arg_count.as.number;

    value_t args[arg_count];
    for (int i = 0; i < arg_count; i++) {
        args[i] = stack_pop();
    }

    value_t func_ip = stack_pop();
    call_stack_push(vm.call_stack, (call_frame_t){.ra = vm.ip, .table = table_init(50)});
    vm.ip = (long)func_ip.as.number;

    for (int i = 0; i < arg_count; i++) {
        stack_push(args[i]);
    }
}

static void run_return() {
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

static void run_jump_if_false() {
    value_t label_index_value = stack_pop();
    value_t boolean_value = stack_pop();

    if (boolean_value.as.boolean == false) {
        long jump_ip = (long)label_index_value.as.number;
        vm.ip = jump_ip;
    }
}

static void run_jump() {
    value_t label_index_value = stack_pop();
    long jump_ip = (long)label_index_value.as.number;
    vm.ip = jump_ip;
}

static void run_add() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot add two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_number(value2.as.number + value1.as.number));
            break;
        }
        case TYPE_STRING: {
            size_t strlen1 = strlen(value1.as.string);
            size_t strlen2 = strlen(value2.as.string);

            char* new_string = (char*)malloc((strlen1 + strlen2) * sizeof(char));

            if (new_string == NULL) {
                error_throw(ERROR_RUNTIME, "Failed to allocate memory for string literal", line());
                return;
            }

            strcpy(new_string, value2.as.string);
            strcat(new_string, value1.as.string);

            stack_push(value_create_string(new_string));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for add", line());
        }
    }
}

static void run_sub() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(value_create_number(value2.as.number - value1.as.number));
}

static void run_mul() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(value_create_number(value2.as.number * value1.as.number));
}

static void run_div() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(value_create_number(value2.as.number / value1.as.number));
}

static void run_cmp_eq() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_eq two values of different datatypes", line());
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
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_eq", line());
        }
    }
}

static void run_cmp_ne() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_ne two values of different datatypes", line());
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
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_ne", line());
        }
    }
}

static void run_cmp_gt() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_gt two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number > value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_gt", line());
        }
    }
}

static void run_cmp_ge() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_ge two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number >= value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_ge", line());
        }
    }
}

static void run_cmp_lt() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_lt two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number < value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_lt", line());
        }
    }
}

static void run_cmp_le() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_le two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(value_create_boolean(value2.as.number <= value1.as.number));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_le", line());
        }
    }
}

static void run_and() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot and two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_BOOLEAN: {
            stack_push(value_create_boolean(value2.as.boolean && value1.as.boolean));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for and", line());
        }
    }
}

static void run_or() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot or two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_BOOLEAN: {
            stack_push(value_create_boolean(value2.as.boolean || value1.as.boolean));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for or", line());
        }
    }
}

static inline void run_print_numeric_literal(value_t* value) {
    if (value->as.number == (int)value->as.number) {
        printf("%d\n", (int)value->as.number);
    } else {
        printf("%.2f\n", value->as.number);
    }
}

static inline void run_print_boolean_literal(value_t* value) {
    printf("%s\n", value->as.boolean ? "true" : "false");
}

static inline void run_print_string_literal(value_t* value) {
    printf("%s\n", value->as.string);
}

static void run_print() {
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
            return error_throw(ERROR_RUNTIME, "Unknown datatype in print statement", line());
        }
    }
}

static void run_stack_clear() {
    value_t stack_item_count = stack_pop();

    for (int i = 0; i < (int)stack_item_count.as.number; i++) {
        stack_pop();
    }
}