#include "vm/value.h"

value_t value_create_number(double number) {
    value_t value;
    value.type = TYPE_NUMBER;
    value.as.number = number;
    return value;
}

value_t value_create_boolean(bool boolean) {
    value_t value;
    value.type = TYPE_BOOLEAN;
    value.as.boolean = boolean;
    return value;
}

value_t value_create_string(char* string) {
    value_t value;
    value.type = TYPE_STRING;
    value.as.string = string;
    return value;
}