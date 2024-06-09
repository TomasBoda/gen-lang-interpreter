#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/common.h"
#include "utils/error.h"

byte_t* double_to_bytes(double value) {
    byte_t* bytes = (byte_t*)malloc(8 * sizeof(byte_t));

    if (bytes == NULL) {
        error_throw(ERROR_RUNTIME, "Failed to allocate bytes for a double", 0);
    }

    memcpy(bytes, &value, sizeof(double));
    return bytes;
}

double bytes_to_double(byte_t* bytes) {
    double value;
    memcpy(&value, bytes, sizeof(double));
    return value;
}

byte_t* uint16_to_bytes(uint16_t value) {
    byte_t* bytes = (byte_t*)malloc(2 * sizeof(byte_t));

    if (bytes == NULL) {
        error_throw(ERROR_RUNTIME, "Failed to allocate bytes for a uint16", 0);
    }

    memcpy(bytes, &value, sizeof(uint16_t));
    return bytes;
}

uint16_t bytes_to_uint16(byte_t* bytes) {
    uint16_t value;
    memcpy(&value, bytes, sizeof(uint16_t));
    return value;
}

byte_t* string_to_bytes(const char* str, size_t* size) {
    size_t str_len = strlen(str);

    byte_t* bytecode = (byte_t*)malloc(str_len + 1);
    if (!bytecode) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    bytecode[0] = (byte_t)str_len;

    memcpy(bytecode + 1, str, str_len);

    *size = str_len + 1;

    return bytecode;
}

char* bytes_to_string(byte_t* bytes, size_t size) {
    char* string = (char*)malloc(size * sizeof(char));

    if (string == NULL) {
        error_throw(ERROR_RUNTIME, "Failed to allocate memory for string literal", 0);
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        string[i] = bytes[i];
    }

    return string;
}

char* substring(const char* str, int length) {
    if (str == NULL || length < 0) {
        return NULL;
    }

    int str_len = strlen(str);

    if (length > str_len) {
        length = str_len;
    }

    char* substr = (char*)malloc(length * sizeof(char));
    if (substr == NULL) {
        return NULL;
    }

    strncpy(substr, str, length);

    return substr;
}
