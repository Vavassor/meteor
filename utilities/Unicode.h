#ifndef UNICODE_H
#define UNICODE_H

#include "CharTypes.h"

#include <stddef.h>

wchar_t* utf8_to_wcs(const char* str, wchar_t* buffer, int count);
char* wcs_to_utf8(const wchar_t* str, char* buffer, int count);

size_t utf8_to_utf16(const char* str, char16_t** data);
size_t utf16_to_utf8(const char16_t* str, char** data);

size_t utf8_surrogate_count(const char* s);
size_t utf16_octet_count(const char16_t* s);
size_t utf8_codepoint_count(const char* s);
size_t utf32_octet_count(const char32_t* s, size_t n);

bool is_locale_utf8(char* locale);

#endif
