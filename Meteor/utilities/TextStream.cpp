#include "TextStream.h"

#include "Parsing.h"

#define BUFFER_SIZE 256

TextStream::TextStream(const char* filePath):
	fileOffset(0),
	bufferOffset(0),
	bufferFilled(0)
{
	file = open_file_stream(filePath);
	buffer = new char[BUFFER_SIZE];
}

TextStream::~TextStream()
{
	close_file_stream(file);
	delete[] buffer;
}

void TextStream::Read_Token(const char* delimiters, String* token)
{
	if(token == nullptr) return;

	char* end = nullptr;
	char* start = next_token(buffer + bufferOffset, delimiters, &end);

	if(end == buffer + bufferFilled)
	{

	}
}

size_t TextStream::Fetch_Data()
{
	size_t bytesRead = read_file_stream(file, fileOffset, (void*) buffer, BUFFER_SIZE);
	bufferFilled = bytesRead;
	fileOffset += bytesRead;
	bufferOffset = 0;
	return bytesRead;
}
