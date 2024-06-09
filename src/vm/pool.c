#include "vm/pool.h"

// TODO: do string interning -> store the same values only once and retrieve the correct index at pool_get()

pool_t* pool_init(uint16_t initial_capacity) {
    pool_t* pool = (pool_t*)malloc(sizeof(pool_t));
    pool->count = 0;
    pool->capacity = initial_capacity;
    pool->values = (value_t *)malloc(sizeof(value_t) * pool->capacity);
    return pool;
}

uint16_t pool_add(pool_t* pool, value_t value) {
    if (pool->count == pool->capacity) {
        pool->capacity *= 2;
        pool->values = (value_t *)realloc(pool->values, sizeof(value_t) * pool->capacity);
    }

    pool->values[pool->count++] = value;
    return pool->count - 1;
}

value_t* pool_get(pool_t* pool, uint16_t index) {
    if (index >= 0 && index < pool->count) {
        return &pool->values[index];
    }
    
    return NULL;
}