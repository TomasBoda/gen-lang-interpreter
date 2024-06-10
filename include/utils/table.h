#ifndef gen_lang_table_h
#define gen_lang_table_h

#include "vm/value.h"

#define TABLE_SIZE 256

/**
 * @brief Object representing a lookup table entry
 * 
 */
typedef struct entry_t {
    char* key;
    value_t value;
    struct entry_t* next;
} entry_t;

/**
 * @brief Object representing a lookup table
 * 
 */
typedef struct {
    int size;
    int capacity;
    entry_t** buckets;
} table_t;

/**
 * @brief Initializes a lookup table object
 * 
 * @param capacity the initial capacity
 * @return table_t* pointer to the initialized lookup table object
 */
table_t* table_init(int capacity);

/**
 * @brief Frees the lookup table object from memory
 * 
 * @param table lookup table object to free
 */
void table_free(table_t* table);

/**
 * @brief Frees the lookup table entry from memory
 * 
 * @param entry lookup table entry to free
 */
void entry_free(entry_t* entry);

/**
 * @brief Stores a new entry (key value pair) in the lookup table
 * 
 * @param table table to store the new entry to
 * @param key key of the entry
 * @param value value of the entry
 */
void table_set(table_t* table, const char* key, value_t value);

/**
 * @brief Retrieves an entry from the lookup table
 * 
 * @param table table to retrieve the entry from
 * @param key key of the entry
 * @return value_t* retrieved entry value (or NULL if the entry does not exist)
 */
value_t* table_get(table_t* table, const char* key);

/**
 * @brief Deletes an entry from the lookup table
 * 
 * @param table table to delete the entry from
 * @param key key of the entry to delete
 */
void table_delete(table_t* table, const char* key);

#endif
