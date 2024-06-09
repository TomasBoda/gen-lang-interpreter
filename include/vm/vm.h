#ifndef gen_lang_vm_h
#define gen_lang_vm_h

#include <stdbool.h>

#include "compiler/bytecode.h"
#include "value.h"

void vm_init(bytecode_t* compiled_bytecode);
void vm_run();

#endif
