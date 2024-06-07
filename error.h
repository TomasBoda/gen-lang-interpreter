#ifndef gen_lang_error_h
#define gen_lang_error_h

#include <stdio.h>
#include <stdlib.h>

typedef enum { error_type_compiler, error_type_runtime } error_type;

void error_throw(error_type type, char* error_string, int line);

#endif
