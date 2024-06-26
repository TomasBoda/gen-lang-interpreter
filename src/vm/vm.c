#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "compiler/bytecode.h"
#include "compiler/compiler.h"
#include "compiler/instruction.h"
#include "utils/common.h"
#include "utils/error.h"
#include "vm/callstack.h"
#include "vm/vm.h"
#include "vm/pool.h"
#include "vm/output.h"

//#define DEBUG
#define TYPE_CHECKING

static bool is_testing = false;

virtual_machine_t vm;
output_t* output;

output_t* vm_get_output() {
    return output;
}

#ifdef DEBUG
static inline void dump_instruction(char* instruction_name) {
    printf("Running %s() on ip %ld\n", instruction_name, vm.ip - 1);
}
#endif

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

static inline value_t number(double number) {
    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = number;
    return value;
}

static inline value_t boolean(bool boolean) {
    value_t value;
    value.type = TYPE_BOOLEAN;
    value.as.boolean = boolean;
    return value;
}

static inline value_t string(char* string) {
    value_t value;
    value.type = TYPE_STRING;
    value.as.string = string;
    return value;
}

static void run_load_const();
static void run_declare_var();
static void run_load_var();
static void run_store_var();
static void run_func_def();
static void run_func_end();
static void run_enum_def();
static void run_enum_end();
static void run_store_enum();
static void run_return();
static void run_call();
static void run_obj_def();
static void run_obj_end();
static void run_new_obj();
static void run_load_prop();
static void run_load_prop_const();
static void run_store_prop();
static void run_init_prop();
static void run_array_def();
static void run_array_get();
static void run_array_set();
static void run_sizeof();
static void run_jump_if_false();
static void run_jump();
static void run_add();
static void run_sub();
static void run_mul();
static void run_div();
static void run_div_floor();
static void run_neg();
static void run_cmp_eq();
static void run_cmp_ne();
static void run_cmp_gt();
static void run_cmp_ge();
static void run_cmp_lt();
static void run_cmp_le();
static void run_and();
static void run_or();
static void run_print();
static void run_endl();
static void run_print_numeric_literal(value_t* value);
static void run_print_boolean_literal(value_t* value);
static void run_print_string_literal(value_t* value);
static void run_stack_clear();

// STACK

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

static inline value_t stack_pop_number() {
    value_t value = stack_pop();

    #ifdef TYPE_CHECKING
    if (value.type != TYPE_NUMBER) {
        error_throw(ERROR_RUNTIME, "Expected stack top to be a number", line());
    }
    #endif

    return value;
}

static inline value_t stack_pop_boolean() {
    value_t value = stack_pop();

    #ifdef TYPE_CHECKING
    if (value.type != TYPE_BOOLEAN) {
        error_throw(ERROR_RUNTIME, "Expected stack top to be a boolean", line());
    }
    #endif

    return value;
}

static inline value_t stack_pop_string() {
    value_t value = stack_pop();
    
    #ifdef TYPE_CHECKING
    if (value.type != TYPE_STRING) {
        error_throw(ERROR_RUNTIME, "Expected stack top to be a string", line());
    }
    #endif

    return value;
}

static inline value_t stack_pop_object() {
    value_t value = stack_pop();
    
    #ifdef TYPE_CHECKING
    if (value.type != TYPE_OBJECT) {
        error_throw(ERROR_RUNTIME, "Expected stack top to be an object", line());
    }
    #endif

    return value;
}

static inline value_t stack_pop_array() {
    value_t value = stack_pop();
    
    #ifdef TYPE_CHECKING
    if (value.type != TYPE_ARRAY) {
        error_throw(ERROR_RUNTIME, "Expected stack top to be an array", line());
    }
    #endif

    return value;
}

// VIRTUAL MACHINE

void vm_init(bytecode_t* bytecode) {
    vm.bytecode = bytecode;
    vm.ip = 0;
    vm.stack_top = vm.stack;

    vm.var_table = table_init(50);
    vm.func_table = table_init(50);
    vm.obj_table = table_init(50);

    vm.call_stack = call_stack_init();
    vm.pool = compiler_get_pool();

    output_free(output);
    output = output_init();
}

void vm_run(bool test) {
    is_testing = test;

    static void* dispatch_table[] = {
        &&label_load_const,             // OP_LOAD_CONST

        &&label_declare_var,            // OP_DECLARE_VAR
        &&label_load_var,               // OP_LOAD_VAR
        &&label_store_var,              // OP_STORE_VAR

        &&label_func_def,               // OP_FUNC_DEF
        &&label_func_end,               // OP_FUNC_END
        &&label_return,                 // OP_RETURN
        &&label_call,                   // OP_CALL

        &&label_enum_def,               // OP_ENUM_DEF
        &&label_store_enum,             // OP_STORE_ENUM
        &&label_enum_end,               // OP_ENUM_END

        &&label_obj_def,                // OP_OBJ_DEF
        &&label_obj_end,                // OP_OBJ_END
        &&label_new_obj,                // OP_NEW_OBJ
        &&label_load_prop,              // OP_LOAD_PROP
        &&label_load_prop_const,        // OP_LOAD_PROP_CONST
        &&label_store_prop,             // OP_STORE_PROP
        &&label_init_prop,              // OP_INIT_PROP

        &&label_array_def,              // OP_ARRAY_DEF
        &&label_array_get,              // OP_ARRAY_GET
        &&label_array_set,              // OP_ARRAY_SET

        &&label_sizeof,                 // OP_SIZE_OF

        &&label_jump,                   // OP_JUMP
        &&label_jump_if_false,          // OP_JUMP_IF_FALSE

        &&label_add,                    // OP_ADD
        &&label_sub,                    // OP_SUB
        &&label_mul,                    // OP_MUL
        &&label_div,                    // OP_DIV
        &&label_div_floor,              // OP_DIV_FLOOR
        &&label_neg,                    // OP_NEG

        &&label_cmp_eq,                 // OP_CMP_EQ
        &&label_cmp_ne,                 // OP_CMP_NE
        &&label_cmp_lt,                 // OP_CMP_LT
        &&label_cmp_le,                 // OP_CMP_LE
        &&label_cmp_gt,                 // OP_CMP_GT
        &&label_cmp_ge,                 // OP_CMP_GE

        &&label_and,                    // OP_AND
        &&label_or,                     // OP_OR

        &&label_print,                  // OP_PRINT
        &&label_endl,                   // OP_ENDL

        &&label_stack_clear,            // OP_STACK_CLEAR
    };

    #define DISPATCH() goto *dispatch_table[next()];

    while (has_next()) {
        DISPATCH();

        label_load_const:
            run_load_const();
            DISPATCH();

        label_declare_var:
            run_declare_var();
            DISPATCH();

        label_load_var:
            run_load_var();
            DISPATCH();

        label_store_var:
            run_store_var();
            DISPATCH();

        label_func_def:
            run_func_def();
            DISPATCH();

        label_func_end:
            run_func_end();
            DISPATCH();

        label_enum_def:
            run_enum_def();
            DISPATCH();

        label_enum_end:
            run_enum_end();
            DISPATCH();

        label_store_enum:
            run_store_enum();
            DISPATCH();

        label_obj_def:
            run_obj_def();
            DISPATCH();

        label_obj_end:
            run_obj_end();
            DISPATCH();

        label_new_obj:
            run_new_obj();
            DISPATCH();

        label_load_prop:
            run_load_prop();
            DISPATCH();

        label_load_prop_const:
            run_load_prop_const();
            DISPATCH();

        label_store_prop:
            run_store_prop();
            DISPATCH();

        label_init_prop:
            run_init_prop();
            DISPATCH();

        label_array_def:
            run_array_def();
            DISPATCH();

        label_array_get:
            run_array_get();
            DISPATCH();

        label_array_set:
            run_array_set();
            DISPATCH();

        label_sizeof:
            run_sizeof();
            DISPATCH();

        label_return:
            run_return();
            if (!has_next()) return;
            DISPATCH();

        label_call:
            run_call();
            DISPATCH();

        label_jump:
            run_jump();
            DISPATCH();

        label_jump_if_false:
            run_jump_if_false();
            DISPATCH();

        label_add:
            run_add();
            DISPATCH();

        label_sub:
            run_sub();
            DISPATCH();

        label_mul:
            run_mul();
            DISPATCH();

        label_div:
            run_div();
            DISPATCH();

        label_div_floor:
            run_div_floor();
            DISPATCH();

        label_neg:
            run_neg();
            DISPATCH();

        label_cmp_eq:
            run_cmp_eq();
            DISPATCH();

        label_cmp_ne:
            run_cmp_ne();
            DISPATCH();

        label_cmp_lt:
            run_cmp_lt();
            DISPATCH();

        label_cmp_le:
            run_cmp_le();
            DISPATCH();

        label_cmp_gt:
            run_cmp_gt();
            DISPATCH();

        label_cmp_ge:
            run_cmp_ge();
            DISPATCH();

        label_and:
            run_and();
            DISPATCH();

        label_or:
            run_or();
            DISPATCH();

        label_print:
            run_print();
            DISPATCH();

        label_endl:
            run_endl();
            DISPATCH();

        label_stack_clear:
            run_stack_clear();
            DISPATCH();
    }
}

// INSTRUCTIONS

static void run_load_const() {
    #ifdef DEBUG
    dump_instruction("run_load_const");
    #endif

    byte_t bytes[2];
    bytes[0] = next();
    bytes[1] = next();

    uint16_t pool_index = bytes_to_uint16(bytes);
    value_t* value = pool_get(vm.pool, pool_index);

    if (value == NULL) {
        error_throw(ERROR_RUNTIME, "Could not fetch constant from the constant pool", line());
        return;
    }

    stack_push(*value);
}

static value_t* load_global_var(char* identifier) {
    value_t* value = table_get(vm.var_table, identifier);

    if (value != NULL) {
        return value;
    }

    value = table_get(vm.func_table, identifier);

    if (value != NULL) {
        return value;
    }

    return NULL;
}

static value_t* load_local_var(char* identifier) {
    if (call_stack_current(vm.call_stack) == NULL) {
        return NULL;
    }

    return table_get(call_stack_current(vm.call_stack)->table, identifier);
}

static value_t* load_var(char* identifier) {
    // check for local variables first
    value_t* local_var = load_local_var(identifier);

    if (local_var != NULL) {
        return local_var;
    }

    // if local variable is not found, check for global variables
    value_t* global_var = load_global_var(identifier);

    if (global_var != NULL) {
        return global_var;
    }

    return NULL;
}

static void run_declare_var() {
    #ifdef DEBUG
    dump_instruction("run_declare_var");
    #endif

    value_t identifier = stack_pop_string();
    value_t value = stack_pop();

    // global variable
    if (call_stack_current(vm.call_stack) == NULL) {
        /* if (table_get(vm.var_table, identifier.as.string) != NULL) {
            error_throw(ERROR_RUNTIME, "Cannot declare a global variable with the same identifier, because it has been already declared", line());
            return;
        } */

        table_set(vm.var_table, identifier.as.string, value);
    }
    // local variable
    else {
        // TODO: fix the removal of local variables within if and while scope
        /* if (table_get(call_stack_current(vm.call_stack)->table, identifier.as.string) != NULL) {
            error_throw(ERROR_RUNTIME, "Cannot declare a local variable with the same identifier, because it has been already declared", line());
            return;
        } */

        table_set(call_stack_current(vm.call_stack)->table, identifier.as.string, value);
    }
}

static void run_load_var() {
    #ifdef DEBUG
    dump_instruction("run_load_var");
    #endif

    value_t identifier = stack_pop_string();
    value_t* value = load_var(identifier.as.string);

    if (value == NULL) {
        error_throw(ERROR_RUNTIME, "Variable with the given identifier does not exist", line());
        return;
    }

    stack_push(*value);
}

static void run_store_var() {
    #ifdef DEBUG
    dump_instruction("run_store_var");
    #endif

    value_t identifier = stack_pop_string();
    value_t value = stack_pop();

    if (table_get(call_stack_current(vm.call_stack)->table, identifier.as.string) != NULL) {
        table_set(call_stack_current(vm.call_stack)->table, identifier.as.string, value);
        return;
    }

    if (table_get(vm.var_table, identifier.as.string) != NULL) {
        table_set(vm.var_table, identifier.as.string, value);
        return;
    }

    error_throw(ERROR_RUNTIME, "Cannot assign to a variable, becuase it does not exist", line());
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
    #ifdef DEBUG
    dump_instruction("run_func_def");
    #endif

    value_t identifier = stack_pop_string();
    table_set(vm.func_table, identifier.as.string, number(vm.ip));

    skip_func_def();
}

static void run_func_end() {
    #ifdef DEBUG
    dump_instruction("run_func_end");
    #endif
}

static void run_enum_def() {
    #ifdef DEBUG
    dump_instruction("run_enum_def");
    #endif

    value_t identifier = stack_pop_string();
    enum_t* enumeration = enum_init();

    if (current() == OP_ENUM_END) {
        error_throw(ERROR_RUNTIME, "Cannot declare an empty enum", line());
        return;
    }

    int index = 0;

    while (current() != OP_ENUM_END) {
        next();
        run_load_const();

        if (next() != OP_STORE_ENUM) {
            error_throw(ERROR_RUNTIME, "OP_STORE_ENUM must follow a OP_LOAD_CONST in enum declaration", line());
            return;
        }

        value_t enum_item = stack_pop_string();
        table_set(enumeration->values, enum_item.as.string, number(index++));
    }

    value_t enum_value;
    enum_value.type = TYPE_ENUM;
    enum_value.as.enumeration = *enumeration;

    // TODO: maybe store enum values to different table? or store them as objects?
    table_set(vm.var_table, identifier.as.string, enum_value);
}

static void run_enum_end() {
    #ifdef DEBUG
    dump_instruction("run_enum_end");
    #endif
}

static void run_store_enum() {
    #ifdef DEBUG
    dump_instruction("run_store_enum");
    #endif
}

static inline void skip_obj_def() {
    while (vm.ip < vm.bytecode->count) {
        switch (current()) {
            case OP_LOAD_CONST: {
                next();
                vm.ip += 2;
                break;
            }
            case OP_OBJ_END: {
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

static void run_obj_def() {
    #ifdef DEBUG
    dump_instruction("run_obj_def");
    #endif

    value_t identifier = stack_pop_string();
    table_set(vm.obj_table, identifier.as.string, number(vm.ip));

    skip_obj_def();
}

static void run_obj_end() {
    #ifdef DEBUG
    dump_instruction("run_obj_end");
    #endif

    call_frame_t* call_frame = call_stack_pop(vm.call_stack);

    if (call_frame == NULL) {
        error_throw(ERROR_RUNTIME, "Cannot pop call frame from the call stack because it is empty", line());
        return;
    }

    vm.ip = call_frame->ra;
    call_frame_free(call_frame);
}

static void run_new_obj() {
    #ifdef DEBUG
    dump_instruction("run_new_obj");
    #endif

    value_t identifier = stack_pop_string();
    value_t* object_ip = table_get(vm.obj_table, identifier.as.string);

    if (object_ip == NULL) {
        error_throw(ERROR_RUNTIME, "Object with the given identifier does not exist", line());
        return;
    }

    value_t object;
    object.type = TYPE_OBJECT;
    object.as.object = *object_init();

    // TODO: initialise table size to 0 ??? (it will most probably not be used)
    call_stack_push(vm.call_stack, (call_frame_t){.ra = vm.ip, .table = table_init(50)});
    vm.ip = (long)object_ip->as.number;

    stack_push(object);
}

static void run_load_prop() {
    #ifdef DEBUG
    dump_instruction("run_load_prop");
    #endif

    value_t identifier = stack_pop_string();
    value_t object = stack_pop_object();

    value_t* property = table_get(object.as.object.properties, identifier.as.string);

    if (property == NULL) {
        error_throw(ERROR_RUNTIME, "Object property with the given identifier does not exist", line());
    }

    stack_push(object);
    stack_push(*property);
}

static void run_load_prop_const() {
    #ifdef DEBUG
    dump_instruction("run_load_prop_const");
    #endif

    value_t identifier = stack_pop_string();
    value_t value = stack_pop();

    switch (value.type) {
        case TYPE_OBJECT: {
            value_t* prop = table_get(value.as.object.properties, identifier.as.string);

            if (prop == NULL) {
                error_throw(ERROR_RUNTIME, "Object property with the given identifier does not exist", line());
            }

            stack_push(*prop);
            break;
        }
        case TYPE_ENUM: {
            value_t* item = table_get(value.as.enumeration.values, identifier.as.string);

            if (item == NULL) {
                error_throw(ERROR_RUNTIME, "Enum item with the given identifier does not exist", line());
            }

            stack_push(*item);
            break;
        }
        default: {
            error_throw(ERROR_RUNTIME, "Unknown datatype in OP_PROP_LOAD_CONST", line());
            return;
        }
    }
}

static void run_store_prop() {
    #ifdef DEBUG
    dump_instruction("run_store_prop");
    #endif

    value_t identifier = stack_pop_string();
    value_t value = stack_pop();
    value_t object = stack_pop_object();

    object_add_property(&(object.as.object), identifier.as.string, value);

    // TODO: fix storing to global variable (this currently only stores to local variable)
    //table_set(call_stack_current(vm.call_stack)->table, identifier.as.string, object);

    if (load_global_var(identifier.as.string) != NULL) {
        table_set(vm.var_table, identifier.as.string, object);
    } else {
        table_set(call_stack_current(vm.call_stack)->table, identifier.as.string, object);
    }
}

static void run_init_prop() {
    #ifdef DEBUG
    dump_instruction("run_init_prop");
    #endif

    value_t identifier = stack_pop_string();
    value_t value = stack_pop();
    value_t object = stack_pop_object();

    object_add_property(&(object.as.object), identifier.as.string, value);

    stack_push(object);
}

static void run_array_def() {
    #ifdef DEBUG
    dump_instruction("run_array_def");
    #endif

    value_t array_size = stack_pop_number();
    array_t* array = array_init((int)array_size.as.number);

    for (int i = 0; i < (int)array_size.as.number; i++) {
        int index = ((int)array_size.as.number) - i - 1;
        array_add_element(array, index, stack_pop());
    }

    value_t array_value;
    array_value.type = TYPE_ARRAY;
    array_value.as.array = *array;

    stack_push(array_value);
}

static void run_array_get() {
    #ifdef DEBUG
    dump_instruction("run_array_get");
    #endif

    value_t index_value = stack_pop_number();
    value_t array_value = stack_pop();

    int index = (int)index_value.as.number;

    if (array_value.type == TYPE_ARRAY) {
        array_t array = array_value.as.array;

        if (index >= array.size) {
            error_throw(ERROR_RUNTIME, "Index out of range", line());
            return;
        }

        value_t element_value = array_get_element(&array, index);
        stack_push(element_value);

        return;
    }

    if (array_value.type == TYPE_STRING) {
        string_t string_value = array_value.as.string;

        if (index >= strlen(string_value)) {
            error_throw(ERROR_RUNTIME, "Index out of range", line());
            return;
        }

        char element_value = string_value[index];
        char* string_array = (char*)malloc(sizeof(char));
        string_array[0] = element_value;

        stack_push(string(string_array));
        return;
    }

    error_throw(ERROR_RUNTIME, "Unsupported value type in OP_ARRAY_GET", line());
}

static void run_array_set() {
    #ifdef DEBUG
    dump_instruction("run_array_set");
    #endif

    value_t value = stack_pop();
    value_t index_value = stack_pop_number();
    value_t array_value = stack_pop_array();

    int index = (int)index_value.as.number;
    array_add_element(&array_value.as.array, index, value);
}

static void run_sizeof() {
    #ifdef DEBUG
    dump_instruction("run_sizeof");
    #endif

    value_t value = stack_pop();

    switch (value.type) {
        case TYPE_STRING: {
            stack_push(number((double)strlen(value.as.string)));
            break;
        }
        case TYPE_ARRAY: {
            stack_push(number((double)value.as.array.size));
            break;
        }
        default: {
            error_throw(ERROR_RUNTIME, "Unsupported datatype in sizeof", line());
            return;
        }
    }
}

static void run_call() {
    #ifdef DEBUG
    dump_instruction("run_call");
    #endif

    value_t func_arg_count = stack_pop_number();
    int arg_count = (int)func_arg_count.as.number;

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
    #ifdef DEBUG
    dump_instruction("run_return");
    #endif

    value_t return_value = stack_pop();
    call_frame_t* call_frame = call_stack_pop(vm.call_stack);

    if (call_frame == NULL) {
        error_throw(ERROR_RUNTIME, "Cannot pop call frame from the call stack because it is empty", line());
        return;
    }

    vm.ip = call_frame->ra;
    stack_push(return_value);

    // exit the virtual machine
    if (call_stack_current(vm.call_stack) == NULL) {
        vm.ip = vm.bytecode->count;
    }

    //call_frame_free(call_frame);
}

static void run_jump_if_false() {
    #ifdef DEBUG
    dump_instruction("run_jump_if_false");
    #endif

    value_t label_index_value = stack_pop_number();
    value_t boolean_value = stack_pop_boolean();

    if (boolean_value.as.boolean == false) {
        long jump_ip = (long)label_index_value.as.number;
        vm.ip = jump_ip;
    }
}

static void run_jump() {
    #ifdef DEBUG
    dump_instruction("run_jump");
    #endif

    value_t label_index_value = stack_pop_number();
    long jump_ip = (long)label_index_value.as.number;
    vm.ip = jump_ip;
}

static void run_add() {
    #ifdef DEBUG
    dump_instruction("run_add");
    #endif

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type == TYPE_ARRAY) {
        array_append(&value2.as.array, value1);
        stack_push(value2);
        return;
    }

    if (value2.type == TYPE_NUMBER && value1.type == TYPE_NUMBER) {
        stack_push(number(value2.as.number + value1.as.number));
        return;
    }

    if (value2.type == TYPE_STRING && value1.type == TYPE_STRING) {
        char* new_string = (char*)malloc((strlen(value2.as.string) + strlen(value1.as.string)) * sizeof(char));

        for (int i = 0; i < strlen(value2.as.string); i++) {
            new_string[i] = value2.as.string[i];
        }

        for (int i = 0; i < strlen(value1.as.string); i++) {
            new_string[i + strlen(value2.as.string)] = value1.as.string[i];
        }

        stack_push(string(new_string));
        return;
    }

    error_throw(ERROR_RUNTIME, "Unknown operands to OP_ADD", line());
}

static void run_sub() {
    #ifdef DEBUG
    dump_instruction("run_sub");
    #endif

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type == TYPE_ARRAY && value1.type == TYPE_NUMBER) {
        array_remove(&value2.as.array, (int)value1.as.number);
        stack_push(value2);
        return;
    }

    if (value2.type == TYPE_NUMBER && value1.type == TYPE_NUMBER) {
        stack_push(number(value2.as.number - value1.as.number));
        return;
    }

    error_throw(ERROR_RUNTIME, "Unknown operands to OP_SUB", line());
}

static void run_mul() {
    #ifdef DEBUG
    dump_instruction("run_mul");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();
    stack_push(number(value2.as.number * value1.as.number));
}

static void run_div() {
    #ifdef DEBUG
    dump_instruction("run_div");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();

    if (value2.as.number == 0) {
        return error_throw(ERROR_RUNTIME, "Division by zero", line());
    }

    stack_push(number(value2.as.number / value1.as.number));
}

static void run_div_floor() {
    #ifdef DEBUG
    dump_instruction("run_div_floor");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();

    if (value2.as.number == 0) {
        return error_throw(ERROR_RUNTIME, "Division by zero", line());
    }

    stack_push(number(floor(value2.as.number / value1.as.number)));
}

static void run_neg() {
    #ifdef DEBUG
    dump_instruction("run_neg");
    #endif

    value_t value = stack_pop();

    switch (value.type) {
        case TYPE_NUMBER: {
            stack_push(number(-value.as.number));
            break;
        }
        case TYPE_BOOLEAN: {
            stack_push(boolean(!value.as.boolean));
            break;
        }
        default: {
            error_throw(ERROR_RUNTIME, "Invalid datatype in negation", line());
            return;
        }
    }
}

static void run_cmp_eq() {
    #ifdef DEBUG
    dump_instruction("run_cmp_eq");
    #endif

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_eq two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(boolean(value2.as.number == value1.as.number));
            break;
        }
        case TYPE_BOOLEAN: {
            stack_push(boolean(value2.as.boolean == value1.as.boolean));
            break;
        }
        case TYPE_STRING: {
            size_t strlen1 = strlen(value1.as.string);
            size_t strlen2 = strlen(value2.as.string);

            if (strlen1 != strlen2) {
                stack_push(boolean(false));
                break;
            }

            bool strings_equal = strcmp(value2.as.string, value1.as.string) == 0;
            stack_push(boolean(strings_equal));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_eq", line());
        }
    }
}

static void run_cmp_ne() {
    #ifdef DEBUG
    dump_instruction("run_cmp_ne");
    #endif

    value_t value1 = stack_pop();
    value_t value2 = stack_pop();

    if (value2.type != value1.type) {
        error_throw(ERROR_RUNTIME, "Cannot cmp_ne two values of different datatypes", line());
        return;
    }

    switch (value1.type) {
        case TYPE_NUMBER: {
            stack_push(boolean(value2.as.number != value1.as.number));
            break;
        }
        case TYPE_BOOLEAN: {
            stack_push(boolean(value2.as.boolean != value1.as.boolean));
            break;
        }
        case TYPE_STRING: {
            size_t strlen1 = strlen(value1.as.string);
            size_t strlen2 = strlen(value2.as.string);

            if (strlen1 != strlen2) {
                stack_push(boolean(true));
                break;
            }

            bool strings_equal = strcmp(value2.as.string, value1.as.string) != 0;
            stack_push(boolean(strings_equal));
            break;
        }
        default: {
            return error_throw(ERROR_RUNTIME, "Unknown datatype for cmp_ne", line());
        }
    }
}

static void run_cmp_gt() {
    #ifdef DEBUG
    dump_instruction("run_cmp_gt");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();
    stack_push(boolean(value2.as.number > value1.as.number));
}

static void run_cmp_ge() {
    #ifdef DEBUG
    dump_instruction("run_cmp_ge");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();
    stack_push(boolean(value2.as.number >= value1.as.number));
}

static void run_cmp_lt() {
    #ifdef DEBUG
    dump_instruction("run_cmp_lt");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();
    stack_push(boolean(value2.as.number < value1.as.number));
}

static void run_cmp_le() {
    #ifdef DEBUG
    dump_instruction("run_cmp_le");
    #endif

    value_t value1 = stack_pop_number();
    value_t value2 = stack_pop_number();
    stack_push(boolean(value2.as.number <= value1.as.number));
}

static void run_and() {
    #ifdef DEBUG
    dump_instruction("run_and");
    #endif

    value_t value1 = stack_pop_boolean();
    value_t value2 = stack_pop_boolean();
    stack_push(boolean(value2.as.boolean && value1.as.boolean));
}

static void run_or() {
    #ifdef DEBUG
    dump_instruction("run_or");
    #endif

    value_t value1 = stack_pop_boolean();
    value_t value2 = stack_pop_boolean();
    stack_push(boolean(value2.as.boolean || value1.as.boolean));
}

static void run_print_newline();
static void run_print_numeric_literal(value_t* value);
static void run_print_boolean_literal(value_t* value);
static void run_print_string_literal(value_t* value);
static void run_print_array(value_t* value);
static void run_print_object(value_t* value);
static void run_print_enum(value_t* value);
static void run_print_any(value_t* value);

static void run_print_newline() {
    printf("\n");
}

static void run_print_numeric_literal(value_t* value) {
    if (value->as.number == (int)value->as.number) {
        printf("%d", (int)value->as.number);
    } else {
        printf("%.2f", value->as.number);
    }
}

static void run_print_boolean_literal(value_t* value) {
    printf("%s", value->as.boolean ? "true" : "false");
}

static void run_print_string_literal(value_t* value) {
    printf("%s", value->as.string);
}

static void run_print_array(value_t* value) {
    printf("[");

    for (int i = 0; i < value->as.array.size; i++) {
        if (value->as.array.elements[i].type == TYPE_STRING) printf("\"");
        run_print_any(&value->as.array.elements[i]);
        if (value->as.array.elements[i].type == TYPE_STRING) printf("\"");

        if (i < value->as.array.size - 1) {
            printf(", ");
        }
    }

    printf("]");
}

static void run_print_object(value_t* value) {
    printf("[object]: not implemented");
}

static void run_print_enum(value_t* value) {
    printf("[enum]: not implemented");
}

static void run_print_any(value_t* value) {
    switch (value->type) {
        case TYPE_NUMBER: {
            return run_print_numeric_literal(value);
        }
        case TYPE_BOOLEAN: {
            return run_print_boolean_literal(value);
        }
        case TYPE_STRING: {
            return run_print_string_literal(value);
        }
        case TYPE_ARRAY: {
            return run_print_array(value);
        }
        case TYPE_OBJECT: {
            return run_print_object(value);
        }
        case TYPE_ENUM: {
            return run_print_enum(value);
        }
    }
}

static void run_print() {
    #ifdef DEBUG
    dump_instruction("run_print");
    #endif

    value_t value = stack_pop();

    if (is_testing) {
        output_add(output, value);
    } else {
        run_print_any(&value);
    }
}

static void run_endl() {
    #ifdef DEBUG
    dump_instruction("run_endl");
    #endif

    if (is_testing) {
        return;
    }
    
    run_print_newline();
}

static void run_stack_clear() {
    #ifdef DEBUG
    dump_instruction("run_stack_clear");
    #endif

    value_t stack_item_count = stack_pop();

    for (int i = 0; i < (int)stack_item_count.as.number; i++) {
        stack_pop();
    }
}