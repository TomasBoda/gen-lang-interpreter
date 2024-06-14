#ifndef gen_lang_common_h
#define gen_lang_common_h

#include <stdlib.h>
#include <stdbool.h>

// FORWARD REFERENCES

typedef struct value_t value_t;
typedef struct table_t table_t;

// TYPEDEFS

typedef uint8_t byte_t;

// DATATYPE TYPEDEFS

typedef double number_t;
typedef bool boolean_t;
typedef char* string_t;
typedef struct {
    table_t* properties;
} object_t;
typedef struct {
    int size;
    value_t* elements;
} array_t;

// VALUE

typedef enum { TYPE_NUMBER, TYPE_BOOLEAN, TYPE_STRING, TYPE_OBJECT, TYPE_ARRAY } value_type;

struct value_t {
    value_type type;
    union {
        number_t number;
        boolean_t boolean;
        string_t string;
        object_t object;
        array_t array;
    } as;
};

array_t* array_init(int size);
void array_add_element(array_t* array, int index, value_t element);
value_t array_get_element(array_t* array, int index);
void array_append(array_t* array, value_t element);
void array_remove(array_t* array, int count);

object_t* object_init();
void object_add_property(object_t* object, char* identifier, value_t element);

// TABLE

typedef struct entry_t {
    char* key;
    value_t value;
    struct entry_t* next;
} entry_t;

struct table_t {
    int size;
    int capacity;
    entry_t** buckets;
};

table_t* table_init(int capacity);
void table_free(table_t* table);
void entry_free(entry_t* entry);

void table_set(table_t* table, const char* key, value_t value);
value_t* table_get(table_t* table, const char* key);
void table_delete(table_t* table, const char* key);

// CONVERSIONS

byte_t* uint16_to_bytes(uint16_t value);
uint16_t bytes_to_uint16(byte_t* bytes);

byte_t* string_to_bytes(const char* str, size_t* size);
char* bytes_to_string(byte_t* bytes, size_t size);

// UTILITIES

char* substring(const char* str, int length);

#endif
