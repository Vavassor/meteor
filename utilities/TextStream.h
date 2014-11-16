#ifndef TEXT_STREAM_H
#define TEXT_STREAM_H

#include "FileHandling.h"
#include "String.h"

#include <stddef.h>

class TextStream
{
public:
	file_handle_t file;
	size_t fileOffset;

	const char* buffer;
	size_t bufferOffset, bufferFilled;

	explicit TextStream(const char* filePath);
	~TextStream();

	void Read_Token(const char* delimiters, String* token);

	size_t Fetch_Data();
};

#endif
