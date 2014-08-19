#ifndef LOGGING_H
#define LOGGING_H

namespace Log
{
	enum LogLevel
	{
		ERR,
		INFO,
		DEBUG,
	};

	void Clear_File();
	void Inc_Time();
	void Write(bool outputToConsole = false);
	void Add(LogLevel level, const char* format, ...);
	const char* GetText();
}

#endif
