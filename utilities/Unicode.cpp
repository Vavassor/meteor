#include "Unicode.h"

wchar_t* utf8_to_wcs(const char* s, wchar_t* buffer, int count)
{
	unsigned char* str = (unsigned char*) s;
	
	--count;
	int i = 0;
	while(*str)
	{
		if(i >= count) return nullptr;

		uint32_t c;

		if(!(*str & 0x80))
		{
			buffer[i++] = *str++;
		}
		else if((*str & 0xE0) == 0xC0)
		{
			if(*str < 0xC2) return nullptr;
			c = (*str++ & 0x1F) << 6;
			if((*str & 0xC0) != 0x80) return nullptr;
			buffer[i++] = c + (*str++ & 0x3F);
		}
		else if((*str & 0xf0) == 0xe0)
		{
			if(*str == 0xE0 && (str[1] < 0xA0 || str[1] > 0xBF)) return nullptr;
			if(*str == 0xED && str[1] > 0x9F) return nullptr;
			c = (*str++ & 0x0F) << 12;
			if((*str & 0xC0) != 0x80) return nullptr;
			c += (*str++ & 0x3F) << 6;
			if((*str & 0xC0) != 0x80) return nullptr;
			buffer[i++] = c + (*str++ & 0x3F);
		}
		else if ((*str & 0xF8) == 0xF0)
		{
			if(*str > 0xF4) return nullptr;
			if(*str == 0xF0 && (str[1] < 0x90 || str[1] > 0xBF)) return nullptr;
			if(*str == 0xF4 && str[1] > 0x8F) return nullptr; // str[1] < 0x80 is checked below

			c = (*str++ & 0x07) << 18;
			if((*str & 0xC0) != 0x80) return nullptr;
			c += (*str++ & 0x3F) << 12;
			if((*str & 0xC0) != 0x80) return nullptr;
			c += (*str++ & 0x3F) << 6;
			if((*str & 0xC0) != 0x80) return nullptr;
			c += (*str++ & 0x3F);

			// utf-8 encodings of values used in surrogate pairs are invalid
			if((c & 0xFFFFF800) == 0xD800) return nullptr;
			if(c >= 0x10000)
			{
				c -= 0x10000;
				if(i + 2 > count) return nullptr;
				buffer[i++] = 0xD800 | (0x3FF & (c >> 10));
				buffer[i++] = 0xDC00 | (0x3FF & c);
			}
		}
		else
		{
			return nullptr;
		}
	}

	buffer[i] = 0;
	return buffer;
}

char* wcs_to_utf8(const wchar_t* str, char* buffer, int count)
{
	--count;
	int i = 0;
	while(*str)
	{
		if(*str < 0x80)
		{
			if(i + 1 > count) return nullptr;
			buffer[i++] = (char) *str++;
		}
		else if(*str < 0x800)
		{
			if(i + 2 > count) return nullptr;
			buffer[i++] = 0xC0 + (*str >> 6);
			buffer[i++] = 0x80 + (*str & 0x3F);
			str += 1;
		}
		else if(*str >= 0xD800 && *str < 0xDC00)
		{
			uint32_t c;
			if(i + 4 > count) return nullptr;
			c = ((str[0] - 0xD800) << 10) + ((str[1]) - 0xDC00) + 0x10000;
			buffer[i++] = 0xF0 +  (c >> 18);
			buffer[i++] = 0x80 + ((c >> 12) & 0x3F);
			buffer[i++] = 0x80 + ((c >>  6) & 0x3F);
			buffer[i++] = 0x80 + ((c      ) & 0x3F);
			str += 2;
		}
		else if(*str >= 0xDC00 && *str < 0xE000)
		{
			return nullptr;
		}
		else
		{
			if(i + 3 > count) return nullptr;
			buffer[i++] = 0xE0 + (*str >> 12);
			buffer[i++] = 0x80 + (*str >> 6 & 0x3F);
			buffer[i++] = 0x80 + (*str & 0x3F);
			str += 1;
		}
	}
	buffer[i] = 0;
	return buffer;
}

size_t utf8_to_utf16(const char* str, char16_t** data)
{
	// determine what the size of string will be after transcoding
	// to make a buffer large enough
	size_t bufferSize = 1 + utf8_surrogate_count(str);
	char16_t* buffer = new char16_t[bufferSize];

	// transcode from utf-8 str to utf-16 buffer
	utf8_to_wcs(str, (wchar_t*) buffer, bufferSize);

	*data = buffer;
	return bufferSize;
}

size_t utf16_to_utf8(const char16_t* str, char** data)
{
	// determine what the size of string will be after transcoding
	// to make a buffer large enough
	size_t bufferSize = 1 + utf16_octet_count(str);
	char* buffer = new char[bufferSize];

	// transcode from utf-16 str to utf-8 buffer
	wcs_to_utf8((wchar_t*)str, buffer, bufferSize);

	*data = buffer;
	return bufferSize;
}

size_t utf8_surrogate_count(const char* s)
{
	int count = 0;
	char* str = (char*) s;
	while(*str)
	{
		if(!(*str & 0x80))             { str += 1; count += 1; }
		else if((*str & 0xE0) == 0xC0) { str += 2; count += 1; }
		else if((*str & 0xf0) == 0xe0) { str += 3; count += 1; }
		else if((*str & 0xF8) == 0xF0) { str += 4; count += 2; }
		else                           { return 0; }
	}
	return count;
}

size_t utf16_octet_count(const char16_t* s)
{
	size_t count = 0;
	char16_t* str = (char16_t*) s;
	while(*str)
	{
		if(*str < 0x80)                          { count += 1; str += 1; }
		else if(*str < 0x800)                    { count += 2; str += 1; }
		else if(*str >= 0xD800 && *str < 0xDC00) { count += 4; str += 2; }
		else if(*str >= 0xDC00 && *str < 0xE000) { return 0; }
		else                                     { count += 3; str += 1; }
	}
	return count;
}

size_t utf8_codepoint_count(const char* s)
{
	size_t count = 0;
	while(*s) count += (*s++ & 0xc0) != 0x80;
	return count;
}

size_t utf32_octet_count(const char32_t* s, size_t n)
{
	size_t count = 0;
	for(size_t i = 0; i < n; ++i)
	{
		if(s[i] < 0x80)         count += 1;
		else if(s[i] < 0x800)   count += 2;
		else if(s[i] < 0x10000) count += 3;
		else                    count += 4;
	}
	return count;
}

int strncmp(const char* s1, const char* s2, size_t n)
{
	while(n--)
	{
		if(*s1++ != *s2++)
			return *(unsigned char*)(s1 - 1) - *(unsigned char*)(s2 - 1);
	}
	return 0;
}

bool is_locale_utf8(char* locale)
{
	for(const char* cp = locale; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; ++cp)
	{
		if(*cp == '.')
		{
			const char* encoding = ++cp;
			for(; *cp != '\0' && *cp != '@' && *cp != '+' && *cp != ','; ++cp);
			if((cp - encoding == 5 && strncmp(encoding, "UTF-8", 5) != 0) ||
			   (cp - encoding == 4 && strncmp(encoding, "utf8", 4)  != 0))
				return true;
			break;
		}
	}
	return false;
}
