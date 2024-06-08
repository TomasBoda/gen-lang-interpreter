#include <stdlib.h>
#include <stdio.h>

#include "utils/error.h"

void error_throw(error_type type, char* error_string, int line) {
    switch (type) {
        case ERROR_COMPILER: {
            printf("GEN Compiler Error (line %d): %s\n", line, error_string);
            break;
        }
        case ERROR_RUNTIME: {
            printf("GEN Runtime Error (line %d): %s\n", line, error_string);
            break;
        }
    }

    exit(1);
}
