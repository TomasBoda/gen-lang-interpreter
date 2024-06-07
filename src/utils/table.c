#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include "vm/vm.h"
#include "utils/table.h"
#include "utils/error.h"

static unsigned long hash_function(const char* str) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash;
}

table_t* table_init(int capacity) {
    table_t* table = (table_t*)malloc(sizeof(table_t*));
    table->capacity = capacity;
    table->size = 0;
    table->buckets = (entry_t**)calloc(capacity, sizeof(entry_t*));

    if (table->buckets == NULL) {
        fprintf(stderr, "Failed to allocate memory for hash table buckets.\n");
        exit(EXIT_FAILURE);
    }

    return table;
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
