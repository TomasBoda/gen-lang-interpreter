#ifndef gen_lang_io_h
#define gen_lang_io_h

char* read_file(const char* filename);
void write_file(const char* filename, const char* content);
void append_file(const char* filename, const char* content);

#endif