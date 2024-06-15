#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
        output->capacity *= 2;
        output->values = (value_t*)realloc(output->values, output->capacity * sizeof(value_t));

        if (output->values == NULL) {
            error_throw(ERROR_COMPILER, "Failed to reallocate memory for output array", 0);
        }
    }

    value_t copied_value = value;

    switch (value.type) {
        case TYPE_STRING: {
            copied_value.type = TYPE_STRING;
            copied_value.as.string = strdup(value.as.string);
            break;
        }
        case TYPE_ARRAY: {
            copied_value.type = TYPE_ARRAY;
            copied_value.as.array = *array_init(value.as.array.size);

            for (int i = 0; i < value.as.array.size; i++) {
                array_add_element(&copied_value.as.array, i, value.as.array.elements[i]);
            }

            break;
        }
        default: {
            copied_value = value;
            break;
        }
    }

    output->values[output->count] = copied_value;
    output->count++;
}

void output_free(output_t* output) {
    if (output != NULL) {
        if (output->values != NULL) {
            free(output->values);
        }
        
        free(output);
    }
}
