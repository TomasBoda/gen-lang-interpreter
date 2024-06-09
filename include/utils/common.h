#ifndef gen_lang_common_h
#define gen_lang_common_h

#include <stdlib.h>

typedef uint8_t byte_t;

byte_t* double_to_bytes(double value);
double bytes_to_double(byte_t* bytes);

byte_t* uint16_to_bytes(uint16_t value);
uint16_t bytes_to_uint16(byte_t* bytes);

byte_t* string_to_bytes(const char* str, size_t* size);
char* bytes_to_string(byte_t* bytes, size_t size);

char* substring(const char* str, int length);

#endif
