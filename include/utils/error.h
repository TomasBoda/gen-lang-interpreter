#ifndef gen_lang_error_h
#define gen_lang_error_h

typedef enum { ERROR_COMPILER, ERROR_RUNTIME } error_type;

void error_throw(error_type type, char* error_string, int line);

#endif
