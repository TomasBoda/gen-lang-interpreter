#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/io.h"
#include "interpreter/interpreter.h"

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: GEN [path]\n");
        exit(64);
    }

    const char* file_path = argv[1];
    char* source_code = read_file(file_path);
    
    interpret(source_code);

    return 0;
}
