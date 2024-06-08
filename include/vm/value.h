#ifndef gen_lang_value_h
#define gen_lang_value_h

#include <stdlib.h>
#include <stdbool.h>

typedef enum { TYPE_NUMBER, TYPE_BOOLEAN, TYPE_STRING } value_type;

typedef struct {
    value_type type;
    union {
        double number;
        bool boolean;
        char* string;
    } as;
} value_t;

value_t value_create_number(double number);
value_t value_create_boolean(bool boolean);
value_t value_create_string(char* string);

#endif