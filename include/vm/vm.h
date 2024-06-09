#ifndef gen_lang_vm_h
#define gen_lang_vm_h

#include <stdbool.h>

#include "compiler/bytecode.h"
#include "value.h"
#include "callstack.h"
#include "utils/table.h"
#include "pool.h"

typedef struct {
    long ip;
    value_t stack[256];
    value_t* stack_top;
    bytecode_t* bytecode;
    table_t* table;
    call_stack_t* call_stack;
    pool_t* pool;
} virtual_machine_t;

void vm_init(bytecode_t* compiled_bytecode);
void vm_run();

#endif
