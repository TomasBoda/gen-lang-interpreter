#ifndef gen_lang_pool_h
#define gen_lang_pool_h

#include "utils/common.h"

/**
 * @brief Object representing a constant pool
 * 
 */
typedef struct {
    uint16_t count;
    uint16_t capacity;
    value_t* values;
} pool_t;

/**
 * @brief Initializes a new constant pool object
 * 
 * @param initial_capacity initial constant pool capacity
 * @return pool_t* pointer to the initialized constant pool
 */
pool_t* pool_init(uint16_t initial_capacity);

/**
 * @brief Adds a new value to the constant pool
 * 
 * @param pool constant pool object to add the new value to
 * @param value value to add to the constant pool object
 * @return uint16_t index of the added value in the constant pool
 */
uint16_t pool_add(pool_t* pool, value_t value);

/**
 * @brief Retrieves a value from the consant pool by its index
 * 
 * @param pool constant pool object to retrieve the value from
 * @param index index of the value to retrieve from the constant pool
 * @return value_t* pointer to the retrieved constant pool value
 */
value_t* pool_get(pool_t* pool, uint16_t index);

#endif