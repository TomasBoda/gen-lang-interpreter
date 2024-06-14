#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "utils/common.h"
#include "utils/error.h"

// VALUE

array_t* array_init(int size) {
    array_t* array = (array_t*)malloc(sizeof(array_t));
    array->size = size;
    array->elements = (value_t*)malloc(size * sizeof(value_t));
    return array;
}

void array_add_element(array_t* array, int index, value_t element) {
    if (index >= array->size) {
        error_throw(ERROR_RUNTIME, "Index out of range in array_add_element", 0);
    }

    array->elements[index] = element;
}

value_t array_get_element(array_t* array, int index) {
    if (index >= array->size) {
        error_throw(ERROR_RUNTIME, "Index out of range in array_get_element", 0);
    }

    return array->elements[index];
}

void array_append(array_t* array, value_t element) {
    array->size++;
    array->elements = (value_t*)realloc(array->elements, array->size * sizeof(value_t));
    array->elements[array->size - 1] = element;
}

void array_remove(array_t* array, int count) {
    if (count >= array->size) {
        array->size = 0;
        free(array->elements);
        array->elements = NULL;
    } else {
        array->size -= count;
        array->elements = (value_t*)realloc(array->elements, array->size * sizeof(value_t));
    }
}

object_t* object_init() {
    object_t* object = (object_t*)malloc(sizeof(object_t));
    object->properties = table_init(50);
    return object;
}

void object_add_property(object_t* object, char* identifier, value_t element) {
    table_set(object->properties, identifier, element);
}

// TABLE

static unsigned long hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

table_t* table_init(int capacity) {
    table_t* table = (table_t*)malloc(sizeof(table_t));
    table->capacity = capacity;
    table->size = 0;
    table->buckets = (entry_t**)calloc(capacity, sizeof(entry_t*));

    if (table->buckets == NULL) {
        fprintf(stderr, "Failed to allocate memory for hash table buckets.\n");
        exit(EXIT_FAILURE);
    }

    return table;
}

void entry_free(entry_t* entry) {
    if (entry == NULL) return;
    free(entry->key);
    if (entry->value.type == TYPE_STRING) {
        free(entry->value.as.string);
    }
    entry_free(entry->next);
    free(entry);
}

void table_free(table_t* table) {
    for (int i = 0; i < table->capacity; i++) {
        entry_free(table->buckets[i]);
    }
    free(table->buckets);
    table->buckets = NULL;
    table->size = 0;
    table->capacity = 0;
}

void table_set(table_t* table, const char* key, value_t value) {
    unsigned long hash = hash_function(key);
    size_t index = hash % table->capacity;

    entry_t* current = table->buckets[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }

        current = current->next;
    }

    entry_t* new_entry = (entry_t*)malloc(sizeof(entry_t));
    if (new_entry == NULL) {
        fprintf(stderr, "Failed to allocate memory for new hash table entry.\n");
        exit(EXIT_FAILURE);
    }

    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;
    table->size++;
}

value_t* table_get(table_t* table, const char* key) {
    unsigned long hash = hash_function(key);
    size_t index = hash % table->capacity;
    entry_t* current = table->buckets[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            return &current->value;
        }

        current = current->next;
    }

    return NULL;
}

void table_delete(table_t* table, const char* key) {
    unsigned long hash = hash_function(key);
    size_t index = hash % table->capacity;

    entry_t* current = table->buckets[index];
    entry_t* prev = NULL;

    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            if (prev == NULL) {
                table->buckets[index] = current->next;
            } else {
                prev->next = current->next;
            }

            free(current->key);
            free(current);
            table->size--;

            return;
        }

        prev = current;
        current = current->next;
    }
}

// CONVERSIONS

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

// UTILITIES

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
