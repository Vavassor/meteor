#include "String.h"

#include "Unicode.h"
#include "StringUtils.h"

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

bool String::Equals(const String& other) const
{
	return compare_strings(sequence, other.sequence) == 0;
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

void String::Append(const char* first, const char* last)
{
	Append(first, last - first);
}

void String::Append(const String& other)
{
	Append(other.sequence, other.size);
}

void String::Reserve(size_t newSize)
{
	if(newSize <= capacity) return;

	// create new, resized buffer
	size_t newCapacity = newSize | 0xF;
	if(capacity / 2 > newCapacity / 3)
		newCapacity = capacity + capacity / 2;

	char* prevSequence = sequence;
	sequence = new char[newCapacity + 1];

	// copy over to new buffer and delete old
	if(prevSequence != nullptr)
	{
		for(size_t i = 0; i < size; ++i)
			sequence[i] = prevSequence[i];
	}
	delete[] prevSequence;

	// reset things to match new capacity
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
