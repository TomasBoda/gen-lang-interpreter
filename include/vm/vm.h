#ifndef gen_lang_vm_h
#define gen_lang_vm_h

#include <stdbool.h>

#include "compiler/bytecode.h"

#define STACK_MAX 256

typedef enum { TYPE_NUMBER, TYPE_BOOLEAN, TYPE_STRING } value_type;

typedef struct {
    value_type type;
    union {
        double number;
        bool boolean;
        char* string;
    } as;
} value_t;

typedef struct {
    byte_t ip;
    value_t stack[STACK_MAX];
    value_t* stack_top;
} virtual_machine_t;

void vm_run(bytecode_t* compiled_bytecode);

void stack_push(value_t value);
value_t stack_pop();

#endif
