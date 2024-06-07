#ifndef gen_lang_table_h
#define gen_lang_table_h

#include "vm.h"

#define TABLE_SIZE 256

typedef struct entry_t {
    char* key;
    value_t value;
    struct entry_t* next;
} entry_t;

typedef struct {
    int size;
    int capacity;
    entry_t** buckets;
} table_t;

table_t* table_init(int capacity);
void table_set(table_t* table, const char* key, value_t value);
value_t* table_get(table_t* table, const char* key);
void table_delete(table_t* table, const char* key);

#endif
