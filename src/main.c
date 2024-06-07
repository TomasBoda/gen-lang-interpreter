#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "interpreter/interpreter.h"

static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);

    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }

    size_t bytes_read = fread(buffer, sizeof(char), fileSize, file);

    if (bytes_read < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }

    buffer[bytes_read] = '\0';
    fclose(file);

    return buffer;
}

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: clox [path]\n");
        exit(64);
    }

    const char* file_path = argv[1];
    char* source_code = read_file(file_path);

    interpreter_init(source_code);
    interpret();

    return 0;
}
