#ifndef FILE_STREAM_H
#define FILE_STREAM_H

#include <stddef.h>

class FileStream
{
public:
	void* fileHandle;
	size_t fileOffset;

	const char* buffer;
	size_t bufferOffset, bufferFilled;

	explicit FileStream(const char* filePath);
	~FileStream();

	size_t Read(char* buffer, size_t size);

	size_t FetchData();
};

#endif
