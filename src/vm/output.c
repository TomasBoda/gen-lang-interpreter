#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "vm/output.h"
#include "utils/error.h"
#include "utils/common.h"

output_t* output_init() {
    output_t* output = (output_t*)malloc(sizeof(output_t));

    if (output == NULL) {
        fprintf(stderr, "Failed to allocate memory for array.\n");
        exit(EXIT_FAILURE);
    }

    output->count = 0;
    output->capacity = OUTPUT_INITIAL_SIZE;
    output->values = (value_t*)malloc(OUTPUT_INITIAL_SIZE * sizeof(value_t));

    if (output->values == NULL) {
        free(output);
        error_throw(ERROR_COMPILER, "Failed to allocate memory for output array", 0);
    }

    return output;
}

void output_add(output_t* output, value_t value) {
    if (output->count == output->capacity) {
        output->capacity += OUTPUT_INITIAL_SIZE;
        output->values = (value_t*)realloc(output->values, output->capacity * sizeof(value_t));

        if (output->values == NULL) {
            error_throw(ERROR_COMPILER, "Failed to reallocate memory for output array", 0);
        }
    }

    output->values[output->count] = value;
    output->count++;
}
