#ifndef gen_lang_stack_h
#define gen_lang_stack_h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "utils/error.h"

typedef struct {
    long data[256];
    long top;
} stack_long_t;

stack_long_t* stack_long_init();

void stack_long_push(stack_long_t* stack, long value);
long stack_long_pop(stack_long_t* stack);
long stack_long_peek(stack_long_t* stack);
bool stack_long_is_empty(stack_long_t* stack);

#endif