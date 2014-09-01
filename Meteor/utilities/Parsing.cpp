#include "Parsing.h"

#include "StringUtils.h"
#include "Logging.h"

#include <cerrno>
#include <climits>
#include <cctype>
#include <cstdlib>

//--- SIMPLE PARSING --------------------------------------------------------------------

int string_to_int(const char* str)
{
	return atoi(str);
}

float string_to_float(const char* str)
{
	return atof(str);
}

double string_to_double(const char* str)
{
	return atof(str);
}

//--- STREAM PROCESSING -----------------------------------------------------------------

int next_unsigned_long_long(const char* str, char** endPtr, unsigned long long* value)
{
	unsigned long long val = strtoull(str, endPtr, 10);

	errno = 0;
	// check if value is in range
	if((errno == ERANGE && val == ULLONG_MAX)
		|| (errno != 0 && val == 0))
	{
		*value = 0;
		return -2;
	}

	// check that we have found any digits
	if(endPtr != nullptr && *endPtr == str)
	{
		*value = 0;
		return -3;
	}

	*value = val;
	return 0;
}

int next_int(const char* str, char** endptr)
{
	unsigned long long value = 0;
	short result = next_unsigned_long_long(str, endptr, &value);
	if(result != 0)
	{
		const char* reason = "";
		switch(result)
		{
			case -2: reason = "value out of range"; break;
			case -3: reason = "no more digits found in string"; break;
		}
		LOG_ISSUE("could not parse integer: %s", reason);
	}
	return value;
}

static char* trim_leading(const char* str)
{
	while(isspace(*str)) ++str;
	return (char*) str;
}

static char* last_non_white(const char* str)
{
	while(*str) ++str;
	do { --str; } while (isspace(*str));
	return (char*) str;
}

char* next_word(const char* str, char** endptr)
{
	char* p = (char*) str;

	p = trim_leading(str);

	// there is no word
	if(*p == '\0')
	{
		if(endptr != nullptr)
			*endptr = (char*) str;
		return nullptr;
	}

	// find end of word and return
	char* wstart = p;
	while(!isspace(*p) && ('\0' != *p)) ++p;
	if(endptr != nullptr) *endptr = p;
	return wstart;
}

//--- SPECIFIC CASE ---------------------------------------------------------------------

static unsigned char char_to_component(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

static unsigned long combine_rgba(unsigned char r, unsigned char g,
	unsigned char b, unsigned char a)
{
	return((unsigned long) r) << 24
		| ((unsigned long) g) << 16
		| ((unsigned long) b) << 8
		| a;
}

unsigned long hex_string_to_rgba(const char* hex)
{
	unsigned char r, g, b, a;
	switch(string_size(hex))
	{
		case 3 + 1:
		{
			// #abc
			r = char_to_component(hex[1]);
			g = char_to_component(hex[2]);
			b = char_to_component(hex[3]);
			a = 0;
			break;
		}
		case 6 + 1:
		{
			// #abcdef
			r = char_to_component(hex[1]) << 4 | char_to_component(hex[2]);
			g = char_to_component(hex[3]) << 4 | char_to_component(hex[4]);
			b = char_to_component(hex[5]) << 4 | char_to_component(hex[6]);
			a = 0;
			break;
		}
		case 8 + 1:
		{
			// #abcdefaa
			r = char_to_component(hex[1]) << 4 | char_to_component(hex[2]);
			g = char_to_component(hex[3]) << 4 | char_to_component(hex[4]);
			b = char_to_component(hex[5]) << 4 | char_to_component(hex[6]);
			a = char_to_component(hex[7]) << 4 | char_to_component(hex[8]);
			break;
		}
		case 12 + 1:
		{
			// #32329999CCCC
			r = char_to_component(hex[1]) << 4 | char_to_component(hex[2]);
			g = char_to_component(hex[5]) << 4 | char_to_component(hex[6]);
			b = char_to_component(hex[9]) << 4 | char_to_component(hex[10]);
			a = 0;
			break;
		}
		default:
		{
			// Return black transparent color
			return combine_rgba(0, 0, 0, 255);
		}
	}
	return combine_rgba(r, g, b, a);
}
