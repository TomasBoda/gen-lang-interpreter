#ifndef gen_lang_output_h
#define gen_lang_output_h

#include <stdio.h>
#include <stdlib.h>

#include "utils/common.h"

#define OUTPUT_INITIAL_SIZE 10

typedef struct {
    int count;
    int capacity;
    value_t* values;
} output_t;

output_t* output_init();

void output_add(output_t* output, value_t value);

#endif
