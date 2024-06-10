#ifndef gen_lang_error_h
#define gen_lang_error_h

/**
 * @brief Error types
 * 
 */
typedef enum { ERROR_COMPILER, ERROR_RUNTIME } error_type;

/**
 * @brief Throws a GEN error and exits the program
 * 
 * @param type type of the error
 * @param error_string error message
 * @param line line in the source code where the error happened
 */
void error_throw(error_type type, char* error_string, int line);

#endif
