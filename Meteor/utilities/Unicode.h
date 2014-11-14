#ifndef UNICODE_H
#define UNICODE_H

#include "CharTypes.h"

#include <stddef.h>

wchar_t* utf8_to_wcs(wchar_t* buffer, const char* ostr, int n);
char* wcs_to_utf8(char* buffer, const wchar_t* str, int n);

size_t utf8_to_utf16(char16_t** data, const char* str);
size_t utf16_to_utf8(char** data, const char16_t* str);

size_t utf8_surrogate_count(const char* s);
size_t utf16_octet_count(const char16_t* s);
size_t utf8_codepoint_count(const char* s);
size_t utf32_octet_count(const char32_t* s, size_t n);

bool is_locale_utf8(char* locale);

#endif
