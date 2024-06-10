#ifndef gen_lang_value_h
#define gen_lang_value_h

#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief Value types
 * 
 */
typedef enum { TYPE_NUMBER, TYPE_BOOLEAN, TYPE_STRING } value_type;

/**
 * @brief Object representing a runtime value
 * 
 */
typedef struct {
    value_type type;
    union {
        double number;
        bool boolean;
        char* string;
    } as;
} value_t;

/**
 * @brief Creates a new runtime value of type number
 * 
 * @param number value to store in the runtime value
 * @return value_t runtime value object
 */
value_t value_create_number(double number);

/**
 * @brief Creates a new runtime value of type boolean
 * 
 * @param number value to store in the runtime value
 * @return value_t runtime value object
 */
value_t value_create_boolean(bool boolean);

/**
 * @brief Creates a new runtime value of type string
 * 
 * @param number value to store in the runtime value
 * @return value_t runtime value object
 */
value_t value_create_string(char* string);

#endif