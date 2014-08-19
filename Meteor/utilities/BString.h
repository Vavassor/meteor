#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include "CharTypes.h"

class String
{
private:
	char* sequence;
	size_t capacity;
	size_t size;

public:
	String();
	String(const String& s);
	String(const char* s);
	~String();

	void Set(const char* s);
	void Set(const char* s, size_t n);

	String& operator = (const String& s);
	String& operator = (const char* s);
	String& operator += (const char* other);
	String& operator += (const String& other);

	friend String operator + (const String& a, const char* b);
	friend String operator + (const char* a, const String& b);
	friend String operator + (const String& a, const String& b);

	bool operator == (const String& other) const;
	bool operator != (const String& other) const;

	void Append(const char* s, size_t n);
	void Append(const char* str);
	void Append(const char32_t* s, size_t n);

	void Reserve(size_t newSize);
	void Clear();

	size_t Count() const;

	size_t Size() const { return size; }
	const char* Data() const { return sequence; }
};

#endif
