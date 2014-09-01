#ifndef PARSING_H
#define PARSING_H

//--- SIMPLE PARSING --------------------------------------------------------------------

int string_to_int(const char* str);
float string_to_float(const char* str);
double string_to_double(const char* str);

//--- STREAM PROCESSING -----------------------------------------------------------------

int next_int(const char* str, char** endptr);
char* next_word(const char* str, char** endptr);

//--- SPECIFIC CASE ---------------------------------------------------------------------

unsigned long hex_string_to_rgba(const char* hex);

#endif
