#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils/io.h"

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        fprintf(stderr, "Could not open file \"%s\".\n", filename);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(fileSize + 1);

    if (buffer == NULL) {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", filename);
        exit(74);
    }

    size_t bytes_read = fread(buffer, sizeof(char), fileSize, file);

    if (bytes_read < fileSize) {
        fprintf(stderr, "Could not read file \"%s\".\n", filename);
        exit(74);
    }

    buffer[bytes_read] = '\0';
    fclose(file);

    return buffer;
}

void write_file(const char* filename, const char* content) {
    FILE *file = fopen(filename, "w");
    
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
}

void append_file(const char* filename, const char* content) {
    FILE *file = fopen(filename, "a");
    
    if (file == NULL) {
        perror("Failed to open file");
        return;
    }
    
    fprintf(file, "%s", content);
    fclose(file);
}