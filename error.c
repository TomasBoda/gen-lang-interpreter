#include "error.h"
#include <stdlib.h>

void error_throw(error_type type, char* error_string, int line) {
    switch (type) {
        case error_type_compiler: {
            printf("GEN Compiler Error (line %d): %s\n", line, error_string);
            break;
        }
        case error_type_runtime: {
            printf("GEN Runtime Error (line %d): %s\n", line, error_string);
            break;
        }
    }

    exit(1);
}
