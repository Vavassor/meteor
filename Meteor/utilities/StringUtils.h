#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <stddef.h>

inline size_t string_size(const char* s)
{
	const char* start = s;
    while(*s++);
    return s - start - 1;
}

inline void reverse_string(char s[], size_t length)
{
	char temp;
	for(int i = 0, j = length - 1; i < j; ++i, --j)
	{
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
	}
}

inline char* copy_string(char* to, const char* from)
{
	char* save = to;
	while(*to++ = *from++);
	return save;
}

inline bool starts_with(const char* s, const char* token)
{
	while(*token)
	{
		if(*token++ != *s++)
			return false;
	}
	return true;
}

inline int compare_strings(const char* a, const char* b)
{
    for(; *a == *b; ++a, ++b)
    {
        if(*a == '\0')
            return 0;
    }
    return *(const unsigned char*)a - *(const unsigned char*)b;
}

#endif
