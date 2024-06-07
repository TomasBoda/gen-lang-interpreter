#ifndef gen_lang_vm_h
#define gen_lang_vm_h

#include <stdbool.h>

#include "compiler/bytecode.h"
#include "value.h"

void vm_init(bytecode_t* compiled_bytecode);
void vm_run();

void stack_push(value_t value);
value_t stack_pop();

#endif
