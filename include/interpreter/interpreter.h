#ifndef gen_lang_interpreter_h
#define gen_lang_interpreter_h

/**
 * @brief Initializes the interpreter
 * 
 * @param source_code source code to interpret
 */
void interpreter_init(const char* source_code);

/**
 * @brief Interpretes the source code and produces output
 * 
 */
void interpret();

#endif
