#include "FileStream.h"

#include <cstring>

#define BUFFER_SIZE 256

FileStream::FileStream(const char* filePath):
	fileOffset(0),
	bufferOffset(0),
	bufferFilled(0)
{
	fileHandle = open_file_stream(filePath);
	buffer = new char[BUFFER_SIZE];
}

FileStream::~FileStream()
{
	close_file_stream(fileHandle);
	delete[] buffer;
}

size_t FileStream::Read(char* data, size_t size)
{
	if(bufferOffset + size > bufferFilled)
	{
		// copy rest of buffer over
		size_t splitSize = bufferFilled - bufferOffset;
		memcpy(data, buffer, splitSize);
		data += splitSize;
		size -= splitSize;

		FetchData();
	}

	// copy from internal buffer
	memcpy(data, buffer + bufferOffset, size);
	bufferOffset += size;

	return size;
}

size_t FileStream::FetchData()
{
	size_t bytesRead = read_file_stream(fileHandle, fileOffset, (void*) buffer, BUFFER_SIZE);
	bufferFilled = bytesRead;
	fileOffset += bytesRead;
	bufferOffset = 0;
	return bytesRead;
}
