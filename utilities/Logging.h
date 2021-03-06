#ifndef LOGGING_H
#define LOGGING_H

namespace Log
{
	enum LogLevel
	{
		ISSUE,
		INFO,
		DEBUG,
	};

	void Clear_File();
	void Inc_Time();
	void Output(bool printToConsole = false);
	void Add(LogLevel level, const char* format, ...);
	const char* Get_Text();
}

#if defined(NDEBUG)
#define DEBUG_LOG(format, ...) // no operation
#elif defined(_DEBUG)
#define DEBUG_LOG(format, ...) Log::Add(Log::DEBUG, (format), ##__VA_ARGS__)
#endif

#define LOG_ISSUE(format, ...) Log::Add(Log::ISSUE, (format), ##__VA_ARGS__)
#define LOG_INFO(format, ...) Log::Add(Log::INFO, (format), ##__VA_ARGS__)

#endif
