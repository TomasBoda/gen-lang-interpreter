#include <stdbool.h>
#include <string.h>

#include "compiler/bytecode.h"
#include "compiler/compiler.h"
#include "compiler/instruction.h"
#include "utils/table.h"
#include "utils/common.h"
#include "utils/error.h"

#define STACK_MAX 256

typedef struct {
    byte_t ip;
    value_t stack[STACK_MAX];
    value_t* stack_top;
    bytecode_t* bytecode;
    table_t* table;
} virtual_machine_t;

virtual_machine_t vm;

static void run_load_const();

static void run_const_numeric_literal();
static void run_const_boolean_literal();
static void run_const_string_literal();

static void run_load_global();
static void run_store_global();

static void run_add();
static void run_sub();
static void run_mul();
static void run_div();

static void run_print();
static void run_print_numeric_literal(value_t* value);
static void run_print_boolean_literal(value_t* value);
static void run_print_string_literal(value_t* value);

/*
static byte_t current() {
    return bytecode->instructions[vm.ip];
}
*/

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
}

void vm_run() {
    while (has_next()) {
        switch (next()) {
            case OP_LOAD_CONST: {
                run_load_const();
                break;
            }
            case OP_CONST_NUMERIC_LITERAL: {
                run_const_numeric_literal();
                break;
            }
            case OP_CONST_BOOLEAN_LITERAL: {
                run_const_boolean_literal();
                break;
            }
            case OP_CONST_STRING_LITERAL: {
                run_const_string_literal();
                break;
            }
            case OP_LOAD_GLOBAL: {
                run_load_global();
                break;
            }
            case OP_STORE_GLOBAL: {
                run_store_global();
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
                break;
            }
        }
    }
}

static void run_load_const() {
    // TODO: nothing to do here
}

static void run_const_numeric_literal() {
    byte_t bytes[8];

    for (size_t i = 0; i < 8; i++) {
        bytes[i] = next();
    }

    double numeric_literal = bytes_to_double(bytes);

    stack_push(create_numeric_literal(numeric_literal));
}

static void run_const_boolean_literal() {
    byte_t boolean_literal = next();
    stack_push(create_boolean_literal(boolean_literal == 0));
}

static void run_const_string_literal() {
    byte_t string_size = next();
    char* string_literal = (char*)malloc(sizeof(char) * string_size);

    for (int i = 0; i < string_size; i++) {
        string_literal[i] = (char)next();
    }

    stack_push(create_string_literal(string_literal));
}

static void run_load_global() {
    value_t identifier = stack_pop();
    value_t* value = table_get(vm.table, identifier.as.string);
    stack_push(*value);
}

static void run_store_global() {
    value_t identifier = stack_pop();
    value_t value = stack_pop();
    table_set(vm.table, identifier.as.string, value);
}

static void run_add() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(error_type_compiler, "Cannot add two values of different datatypes", 0);
    }

    if (value2.type == TYPE_NUMBER && value1.type == TYPE_NUMBER) {
        stack_push(create_numeric_literal(value2.as.number + value1.as.number));
        return;
    }

    if (value2.type == TYPE_STRING && value1.type == TYPE_STRING) {
        size_t strlen1 = strlen(value1.as.string);
        size_t strlen2 = strlen(value2.as.string);

        char* new_string = (char*)malloc((strlen1 + strlen2) * sizeof(char));

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
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(create_numeric_literal(value2.as.number - value1.as.number));
}

static void run_mul() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(create_numeric_literal(value2.as.number * value1.as.number));
}

static void run_div() {
    value_t value1 = stack_pop();
    value_t value2 = stack_pop();
    stack_push(create_numeric_literal(value2.as.number / value1.as.number));
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
    }
}

static void run_print_numeric_literal(value_t* value) {
    if (value->as.number == (int)value->as.number) {
        printf("%d\n", (int)value->as.number);
    } else {
        printf("%.2f\n", value->as.number);
    }
}

static void run_print_boolean_literal(value_t* value) {
    printf("%s\n", value->as.boolean ? "true" : "false");
}

static void run_print_string_literal(value_t* value) {
    printf("%s\n", value->as.string);
}