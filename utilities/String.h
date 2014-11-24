#ifndef STRING_H
#define STRING_H

#include <stddef.h>

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

	void Append(const String& other);
	void Append(const char* s, size_t n);
	void Append(const char* s);
	void Append(const char* first, const char* last);
	
	bool Equals(const String& other) const;

	void Reserve(size_t newSize);
	void Clear();

	size_t Count() const;

	size_t Size() const { return size; }
	const char* Data() const { return sequence; }
};

#endif
