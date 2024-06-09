#ifndef gen_lang_pool_h
#define gen_lang_pool_h

#include "vm/value.h"

typedef struct {
    uint16_t count;
    uint16_t capacity;
    value_t* values;
} pool_t;

pool_t* pool_init(uint16_t initial_capacity);
uint16_t pool_add(pool_t* pool, value_t value);
value_t* pool_get(pool_t* pool, uint16_t index);

#endif