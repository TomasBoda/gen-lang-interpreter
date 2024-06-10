#ifndef gen_lang_vm_h
#define gen_lang_vm_h

#include <stdbool.h>

#include "compiler/bytecode.h"
#include "value.h"
#include "callstack.h"
#include "utils/table.h"
#include "pool.h"

/**
 * @brief Object representing a virtual machine
 * 
 */
typedef struct {
    long ip;
    value_t stack[256];
    value_t* stack_top;
    bytecode_t* bytecode;
    table_t* table;
    call_stack_t* call_stack;
    pool_t* pool;
} virtual_machine_t;

/**
 * @brief Initializes the virtual machine object
 * 
 * @param compiled_bytecode bytecode object to run using the virtual machine
 */
void vm_init(bytecode_t* compiled_bytecode);

/**
 * @brief Starts the virtual machine and interprets the bytecode
 * 
 */
void vm_run();

#endif
