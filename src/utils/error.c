#include <stdlib.h>
#include <stdio.h>

#include "utils/error.h"

void error_throw(error_type type, char* error_string, int line) {
    switch (type) {
        case ERROR_COMPILER: {
            printf("\033[31mGEN Compiler Error (line %d): %s\033[0m\n", line, error_string);
            break;
        }
        case ERROR_RUNTIME: {
            printf("\033[31mGEN Runtime Error (line %d): %s\033[0m\n", line, error_string);
            break;
        }
    }

    exit(1);
}
