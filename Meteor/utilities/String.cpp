#include "String.h"

#include "UnicodeUtils.h"

static size_t string_size(const char* s)
{
	const char* start = s;
    while(*s++);
    return s - start - 1;
}

String::String():
	sequence(nullptr),
	capacity(0),
	size(0)
{}

String::String(const String& s):
	sequence(nullptr),
	capacity(0),
	size(0)
{
	Set(s.sequence, s.size);
}

String::String(const char* s):
	sequence(nullptr),
	capacity(0),
	size(0)
{
	Set(s);
}

String::~String()
{
	delete[] sequence;
}

void String::Set(const char* s)
{
	Set(s, string_size(s));
}

void String::Set(const char* s, size_t n)
{
	Reserve(n);
	for(size_t i = 0; i < n; ++i)
		sequence[i] = s[i];
	sequence[n] = '\0';
	size = n;
}

String& String::operator = (const String& s)
{
	if(this != &s) Set(s.sequence, s.size);
	return *this;
}

String& String::operator = (const char* s)
{
	if(sequence != s) Set(s);
	return *this;
}

String& String::operator += (const String& other)
{
	Append(other.sequence, other.size);
	return *this;
}

String& String::operator += (const char* other)
{
	Append(other);
	return *this;
}

String operator + (const String& a, const char* b)
{
	String str(a);
	str += b;
	return str;
}

String operator + (const char* a, const String& b)
{
	String str(a);
	str += b;
	return str;
}

String operator + (const String& a, const String& b)
{
	String str(a);
	str += b;
	return str;
}

bool String::operator == (const String& other) const
{
	if(size != other.Size()) return false;

	const char* otherSequence = other.Data();
	for(int i = 0, n = string_size(sequence); i < n; ++i)
	{
		if(sequence[i] != otherSequence[i]) return false;
	}
	return true;
}

bool String::operator != (const String& other) const
{
	return !operator==(other);
}

void String::Append(const char* s, size_t n)
{
	Reserve(size + n);
	for(size_t i = 0; i < n; ++i)
		sequence[size + i] = s[i];
	size += n;
	sequence[size] = '\0';
}

void String::Append(const char* s)
{
	Append(s, string_size(s));
}

void String::Append(const char32_t* s, size_t n)
{
	size_t sizeUTF8 = utf32_octet_count(s, n);
	Reserve(size + sizeUTF8);

	for(size_t i = 0; i < n; ++i)
	{
		if(s[i] < 0x80)
		{
			sequence[size++] = s[i];
		}
		else if(s[i] < 0x800)
		{
			sequence[size++] = s[i] >> 6			| 0xC0;
			sequence[size++] = s[i]			& 0x3F	| 0x80;
		}
		else if(s[i] < 0x10000)
		{
			sequence[size++] = s[i] >> 12			| 0xE0;
			sequence[size++] = s[i] >> 6	& 0x3F	| 0x80;
			sequence[size++] = s[i]			& 0x3F	| 0x80;
		}
		else
		{
			sequence[size++] = s[i] >> 18			| 0xF0;
			sequence[size++] = s[i] >> 12	& 0x3F	| 0x80;
			sequence[size++] = s[i] >> 6	& 0x3F	| 0x80;
			sequence[size++] = s[i]			& 0x3F	| 0x80;
		}
	}
	sequence[size] = '\0';
}

void String::Reserve(size_t newSize)
{
	if(newSize <= capacity) return;

	size_t newCapacity = newSize | 0xF;
	if(capacity / 2 > newCapacity / 3)
		newCapacity = capacity + capacity / 2;

	char* prevSequence = sequence;
	sequence = new char[newCapacity + 1];

	if(prevSequence != nullptr)
	{
		for(size_t i = 0; i < size; ++i)
			sequence[i] = prevSequence[i];
	}
	delete[] prevSequence;

	sequence[newCapacity] = '\0';
	capacity = newCapacity;
}

void String::Clear()
{
	delete[] sequence;

	sequence = nullptr;
	size = 0;
	capacity = 0;
}

size_t String::Count() const
{
	return utf8_codepoint_count(sequence);
}
