#include "Conversion.h"

#include <math.h>
#include <float.h>
#include <cstdlib>

#include "Maths.h"

#define NAN_TEXT		"NaN"
#define INFINITY_TEXT	"infinity"

static inline void reverse_string(char s[], size_t length)
{
	char temp;
	for(int i = 0, j = length - 1; i < j; ++i, --j)
	{
		temp = s[i];
		s[i] = s[j];
		s[j] = temp;
	}
}

static inline void copy_string(char* to, const char* from)
{
	while(*from != '\0') *to++ = *from++;
	*to = '\0';
}

void bool_to_string(bool a, char* str)
{
	copy_string(str, (a) ? "true" : "false");
}

void int_to_string(long long value, char* str, unsigned base)
{
	if(base < 2 || base > 36) base = 10;
	if(base == 10 && value < 0)
	{
		*str++ = '-';
		value = -value;
	}

	// generate digits in order of least to most significant
	int i = 0;
	do
	{
		int digit = value % base;
		if(digit < 0xA)
			str[i++] = '0' + digit;
		else
			str[i++] = 'A' + digit - 0xA;
	}
	while(value /= base);

	str[i] = '\0';
	
	// make it read correctly from most to least significant digits
	reverse_string(str, i);
}

void float_to_string(double n, char* str, unsigned precision)
{
	// handle special cases
	if(is_nan(n))
	{
		copy_string(str, NAN_TEXT);
	}
	else if(is_infinite(n))
	{
		copy_string(str, INFINITY_TEXT);
	}
	else if(n == 0.0)
	{
		copy_string(str, "0");
	}
	else
	{
		// handle ordinary case
		char* c = str;
		bool neg = n < 0;
		if(neg) n = -n;

		// calculate magnitude
		int m = log10(n);
		bool useExp = m >= 14 || neg && m >= 9 || m <= -9;
		if(neg) *(c++) = '-';
        
		// set up for scientific notation
		int m1;
		if(useExp)
		{
			if(m < 0) m -= 1.0;
			n /= pow(10.0, m);
			m1 = m;
			m = 0;
		}
		if(m < 1.0)
		{
			m = 0;
		}

		// convert the number
		double invPrecision = pow(0.1, (double) precision);
		while(n > invPrecision || m >= 0)
		{
			double weight = pow(10.0, m);
			if(weight > 0 && !is_infinite(weight))
			{
				int digit = floor(n / weight);
				n -= digit * weight;
				*(c++) = '0' + digit;
			}
			if(m == 0 && n > 0)
				*(c++) = '.';
			--m;
		}
		if(useExp)
		{
			// convert the exponent
			*(c++) = 'e';
			if(m1 > 0)
			{
				*(c++) = '+';
			}
			else
			{
				*(c++) = '-';
				m1 = -m1;
			}

			m = 0;
			while(m1 > 0)
			{
				*(c++) = '0' + m1 % 10;
				m1 /= 10;
				++m;
			}
			c -= m;

			reverse_string(c, m);
			c += m;
		}
		*c = '\0';
	}
}

struct SplitFloat
{
	char sign					: 1;
	char exponent				: 8;
	unsigned long significand	: 24;
};

struct SplitDouble
{
	char sign						: 1;
	short exponent					: 11;
	unsigned long long significand	: 52;
};

static void float_decompose(float n, SplitFloat* f)
{
	unsigned long data = *(unsigned long*)(&n);

	f->significand = (data & 0xFFFFFF) << 1;
	data >>= 23;
	f->exponent = (data & 0xFF) - 127;
	data >>= 8;
	f->sign = data & 0x1;
}

static void double_decompose(double n, SplitDouble* d)
{
	unsigned long long data = *(unsigned long long*)(&n);

	d->significand = data & 0xFFFFFFFFFFFFF;
	data >>= 52;
	d->exponent = (data & 0x7FF) - 1023;
	data >>= 11;
	d->sign = data & 0x1;
}

void float_to_hex_string(float n, char* str)
{
	// handle special cases
	if(is_nan(n))
	{
		copy_string(str, NAN_TEXT);
	}
	else if(is_infinite(n))
	{
		copy_string(str, INFINITY_TEXT);
	}
	else if(n == 0.0f)
	{
		copy_string(str, "0");
	}
	else
	{
		// handle ordinary case
		SplitFloat f;
		float_decompose(n, &f);

		if(f.sign < 0) *str++ = '-';
		copy_string(str, "0x1.");
		str += 4;

		int_to_string(f.significand, str, 16);
		while(*++str);

		*str++ = 'p';
		if(f.exponent >= 0) *str++ = '+';
		int_to_string(f.exponent, str, 10);
	}
}

void double_to_hex_string(double n, char* str)
{
	// handle special cases
	if(is_nan(n))
	{
		copy_string(str, NAN_TEXT);
	}
	else if(is_infinite(n))
	{
		copy_string(str, INFINITY_TEXT);
	}
	else if(n == 0.0)
	{
		copy_string(str, "0");
	}
	else
	{
		SplitDouble d;
		double_decompose(n, &d);

		if(d.sign < 0) *str++ = '-';
		copy_string(str, "0x1.");
		str += 4;

		int_to_string(d.significand, str, 16);
		while(*++str);

		*str++ = 'p';
		if(d.exponent >= 0) *str++ = '+';
		int_to_string(d.exponent, str, 10);
	}
}

// ------------------------------------------------------------------------------------------------

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
