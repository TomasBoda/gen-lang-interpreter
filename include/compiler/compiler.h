#ifndef gen_lang_compiler_h
#define gen_lang_compiler_h

#include <stdio.h>
#include <stdlib.h>

#include "bytecode.h"

void compiler_init();
bytecode_t* compile();

#endif
