#ifndef gen_lang_value_h
#define gen_lang_value_h

typedef enum { TYPE_NUMBER, TYPE_BOOLEAN, TYPE_STRING } value_type;

typedef struct {
    value_type type;
    union {
        double number;
        bool boolean;
        char* string;
    } as;
} value_t;

#endif