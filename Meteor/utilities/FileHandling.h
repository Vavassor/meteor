#ifndef FILE_HANDLING_H
#define FILE_HANDLING_H

#include <stddef.h>

enum FileWriteMode
{
	FILE_MODE_OVERWRITE,
	FILE_MODE_APPEND,
	FILE_MODE_CLEAR,
	FILE_MODE_READ,
};

size_t load_binary_file(void** data, const char* filePath);
void save_binary_file(const void* data, size_t size, const char* filePath, FileWriteMode writeMode = FILE_MODE_OVERWRITE);

void clear_file(const char* filePath);

void save_text_file(const char* data, size_t size, const char* filePath, FileWriteMode writeMode = FILE_MODE_OVERWRITE);

void* open_file_stream(const char* filePath);
void close_file_stream(void* fileHandle);
size_t read_file_stream(void* fileHandle, unsigned long long readOffset, void* buffer, size_t size);

#endif
