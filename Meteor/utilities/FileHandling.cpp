#include "FileHandling.h"

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__)
#include <errno.h>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#endif

#include "UnicodeUtils.h"
#include "Logging.h"

#if defined(_WIN32)

static HANDLE open_file(const char* filePath, FileWriteMode writeMode)
{
	// convert file path to utf-16 for windows
	wchar_t* widePath = nullptr;
	size_t size = utf8_to_utf16((char16_t**) &widePath, filePath);
	if(size <= 0)
	{
		Log::Add(Log::ERR, "%s%s", "could not convert file path to open file: ", filePath);

		delete[] widePath;
		return INVALID_HANDLE_VALUE;
	}

	// setup file-opening options
	DWORD access, shareMode, disposition;
	DWORD attributes = FILE_ATTRIBUTE_NORMAL;
	DWORD flags = 0;
	switch(writeMode)
	{
		default:
		case FILE_MODE_OVERWRITE:
		{
			access = GENERIC_WRITE;
			shareMode = 0;
			disposition = CREATE_ALWAYS;
			break;
		}
		case FILE_MODE_APPEND:
		{
			access = FILE_APPEND_DATA;
			shareMode = FILE_SHARE_READ;
			disposition = OPEN_ALWAYS;
			break;
		}
		case FILE_MODE_CLEAR:
		{
			access = GENERIC_WRITE;
			shareMode = 0;
			disposition = TRUNCATE_EXISTING;
			break;
		}
		case FILE_MODE_READ:
		{
			access = GENERIC_READ;
			shareMode = FILE_SHARE_READ;
			disposition = OPEN_EXISTING;
			attributes = FILE_ATTRIBUTE_READONLY;
			flags = FILE_FLAG_SEQUENTIAL_SCAN;
			break;
		}
	}

	HANDLE file = CreateFile(widePath, access, shareMode, NULL, disposition, flags | attributes, NULL);
	if(file == INVALID_HANDLE_VALUE)
	{
		Log::Add(Log::ERR, "%s%s", "could not open file: ", filePath);
	}

	delete[] widePath;

	return file;
}

size_t load_binary_file(void** data, const char* filePath)
{
	HANDLE file = open_file(filePath, FILE_MODE_READ);
	if(file == INVALID_HANDLE_VALUE) return 0;

	BY_HANDLE_FILE_INFORMATION fileInfo = {};
    BOOL gotFileInfo = GetFileInformationByHandle(file, &fileInfo);
	if(gotFileInfo == FALSE)
	{
		Log::Add(Log::ERR, "%s%s", "could not obtain file info for file: ", filePath);

		CloseHandle(file);
		return 0;
	}

	DWORD size = fileInfo.nFileSizeLow;
	char* buffer = new char[size];

	DWORD numBytesRead;
	BOOL fileRead = ReadFile(file, buffer, size, &numBytesRead, NULL);
	if(fileRead == FALSE || numBytesRead == 0)
	{
		Log::Add(Log::ERR, "%s%s", "could not read data from file: ", filePath);

		CloseHandle(file);
		delete[] buffer;
		return 0;
	}

	CloseHandle(file);

	*data = buffer;
	return numBytesRead;
}

void save_binary_file(const void* data, size_t size, const char* filePath, FileWriteMode writeMode)
{
	HANDLE file = open_file(filePath, writeMode);

	DWORD numBytesWritten;
	BOOL fileWrote = WriteFile(file, data, size, &numBytesWritten, NULL);
	if(fileWrote == FALSE || numBytesWritten == 0)
	{
		Log::Add(Log::ERR, "%s%s", "could not write data to file: ", filePath);
	}

	CloseHandle(file);
}

void clear_file(const char* filePath)
{
	HANDLE file = open_file(filePath, FILE_MODE_CLEAR);
	CloseHandle(file);
}

void save_text_file(const char* data, size_t size, const char* filePath, FileWriteMode writeMode)
{
	HANDLE file = open_file(filePath, FILE_MODE_OVERWRITE);

	char byteOrderMark[3] = { 0xEF, 0xBB, 0xBF };

	DWORD numBytesWritten;
	BOOL fileWrote = WriteFile(file, byteOrderMark, ARRAYSIZE(byteOrderMark), &numBytesWritten, NULL);
	if(fileWrote == FALSE || numBytesWritten == 0)
	{
		Log::Add(Log::ERR, "%s%s", "could not write BOM to file: ", filePath);
	}

	OVERLAPPED overlap = {};
	overlap.Offset = 0xFFFFFFFF;
	overlap.OffsetHigh = 0xFFFFFFFF;

	fileWrote = WriteFile(file, data, size, &numBytesWritten, &overlap);
	if(fileWrote == FALSE || numBytesWritten == 0)
	{
		Log::Add(Log::ERR, "%s%s", "could not write text to file: ", filePath);
	}

	CloseHandle(file);
}

void* open_file_stream(const char* filePath)
{
	return open_file(filePath, FILE_MODE_READ);
}

void close_file_stream(void* stream)
{
	CloseHandle(stream);
}

size_t read_file_stream(void* fileHandle, unsigned long long readOffset, void* buffer, size_t size)
{
	OVERLAPPED overlap = {};
	overlap.Offset = readOffset & 0xFFFFFFFF;
	overlap.OffsetHigh = readOffset >> 8;

	DWORD numBytesRead;
	BOOL fileRead = ReadFile(fileHandle, buffer, size, &numBytesRead, &overlap);
	if(fileRead == FALSE || numBytesRead == 0)
	{
		Log::Add(Log::ERR, "%s", "could not read data from file stream.");
	}

	return numBytesRead;
}

#elif defined(__unix__)

static int open_file(const char* filePath, FileWriteMode openMode)
{
	int flags;
	mode_t mode;
	switch(openMode)
	{
		case FILE_MODE_OVERWRITE:
		{
			flags = O_WRONLY | O_EXLOCK |
					O_CREAT |
					O_TRUNC;
			mode = 0666;
			break;
		}
		case FILE_MODE_APPEND:
		{
			flags = O_WRONLY |
					O_CREAT |
					O_APPEND;
			mode = 0666;
			break;
		}
		case FILE_MODE_CLEAR:
		{
			flags = O_WRONLY | O_EXLOCK |
					O_TRUNC;
			break;
		}
		case FILE_MODE_READ:
		{
			flags = O_RDONLY | O_SHLOCK;
			break;
		}
	}

	int file = open(filePath, flags, mode);
	if(file < 0)
	{
		Log::Add(Log::ERR, "%s%s%s%s", "Error opening file ", filePath, " - ", strerror(errno));
		return 0;
	}

	return file;
}

size_t load_binary_file(void** data, const char* filePath)
{
	int file = open_file(filePath, FILE_MODE_READ);
	if(file < 0) return 0;

	struct stat info;
	int result = fstat(file, &info);
	if(result < 0)
	{
		Log::Add(Log::ERR, "%s%s%s%s", "Error reading file ", filePath, " - ", strerror(errno));

		close(file);
		return 0;
	}

	off_t size = info.st_size;
	char* buffer = new char[size];

	ssize_t numReadBytes = read(file, buffer, size);
	if(numReadBytes < 0)
	{
		Log::Add(Log::ERR, "%s%s%s%s", "Error reading file ", filePath, " - ", strerror(errno));

		close(file);
		return 0;
	}

	close(file);

	*data = buffer;
	return numReadBytes;
}

void save_binary_file(const void* data, size_t size, const char* filePath, FileWriteMode writeMode)
{
	int file = open_file(filePath, writeMode);

	ssize_t bytesWritten = write(file, data, size);
	if(bytesWritten < 0)
	{
		Log::Add(Log::ERR, "%s%s%s%s", "Could not write to file ", filePath, " - ", strerror(errno));
	}

	close(file);
}

void clear_file(const char* filePath)
{
	int file = open_file(filePath, FILE_MODE_CLEAR);
	close(file);
}

#endif
