#ifndef LOGGING_H
#define LOGGING_H

namespace Log
{
	enum Level
	{
		ISSUE,
		INFO,
		DEBUG,
	};

	void Clear_File();
	void Inc_Time();
	void Output(bool printToConsole = false);
	void Add(Level level, const char* format, ...);
	const char* Get_Text();
}

#if defined(_DEBUG)
#define DEBUG_LOG(format, ...) Log::Add(Log::Level::DEBUG, (format), ##__VA_ARGS__)
#else
#define DEBUG_LOG(format, ...) // no operation
#endif

#define LOG_ISSUE(format, ...) Log::Add(Log::Level::ISSUE, (format), ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::Add(Log::Level::INFO, (format), ##__VA_ARGS__)

#endif
