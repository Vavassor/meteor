#include "Logging.h"

#include <time.h>
#include <stdarg.h>

#if defined(_MSC_VER) && defined(_WIN32)
#include <Windows.h>
#else
#include <cstdio>
#endif

#include "BString.h"
#include "FileHandling.h"
#include "Conversion.h"

#define LOG_FILE_NAME "log_file.txt"

namespace Log
{
	String stream;
	long ticks;

	enum ParseSizeParameter
	{
		PARSE_NORMAL,
		PARSE_BYTE,
		PARSE_SHORT,
		PARSE_LONG,
		PARSE_LONG_LONG,
	};
}

void Log::Clear_File()
{
	clear_file(LOG_FILE_NAME);
}

void Log::Inc_Time()
{
	ticks++;
}

void Log::Write(bool outputToConsole)
{
	if(stream.Size() == 0) return;

	#if defined(_DEBUG)
	if(outputToConsole)
	{
		#if defined(_MSC_VER) && defined(_WIN32)
			OutputDebugStringA(stream.Data());
		#else
			printf("%s", stream.Data());
		#endif
	}
	#endif

	save_text_file(stream.Data(), stream.Size(), LOG_FILE_NAME, FILE_MODE_APPEND);

	stream.Clear();
}

static const char* log_level_name(Log::LogLevel level)
{
	switch(level)
	{
		case Log::ERR:		return "ERROR";
		case Log::INFO:		return "INFO";
		case Log::DEBUG:	return "DEBUG";
	}
	return '\0';
}

#define LOG_CASE(CASE_TYPE, ParamType, convert_function)\
	case CASE_TYPE:										\
	{													\
		ParamType param = va_arg(arguments, ParamType);	\
		convert_function(param, str);					\
		break;											\
	}

void Log::Add(LogLevel level, const char* format, ...)
{
	// append log header to line
	stream += "LOG-";

	time_t signature = time(NULL);
	tm* t = localtime(&signature);

	char timeStr[20];
	int_to_string(t->tm_hour, timeStr);
	stream += timeStr;
	stream += ":";

	int_to_string(t->tm_min, timeStr);
	stream += timeStr;
	stream += ":";

	int_to_string(t->tm_sec, timeStr);
	stream += timeStr;
	stream += "/";

	int_to_string(ticks, timeStr);
	stream += timeStr;
	stream += " ";

	stream += log_level_name(level);
	stream += ": ";

	// write parameter data to log
	va_list arguments;
	va_start(arguments, format);

	ParseSizeParameter sizeType = PARSE_NORMAL;

	char* s = (char*) format;
	while(*s)
	{
		// number types
		if(*s == 'i')
		{
			char str[20];
			switch(sizeType)
			{
				LOG_CASE(PARSE_BYTE, char, int_to_string)
				LOG_CASE(PARSE_SHORT, short, int_to_string)
				LOG_CASE(PARSE_NORMAL, int, int_to_string)
				LOG_CASE(PARSE_LONG, long, int_to_string)
				LOG_CASE(PARSE_LONG_LONG, long long, int_to_string)
			}
			stream.Append(str, 20);
		}
		else if(*s == 'u')
		{
			char str[20];
			switch(sizeType)
			{
				LOG_CASE(PARSE_BYTE, unsigned char, int_to_string)
				LOG_CASE(PARSE_SHORT, unsigned short, int_to_string)
				LOG_CASE(PARSE_NORMAL, unsigned int, int_to_string)
				LOG_CASE(PARSE_LONG, unsigned long, int_to_string)
				LOG_CASE(PARSE_LONG_LONG, unsigned long long, int_to_string)
			}
		}
		else if(*s == 'f')
		{
			char str[32];
			switch(sizeType)
			{
				case PARSE_BYTE:
				case PARSE_SHORT:
				case PARSE_NORMAL:
				{
					float param = va_arg(arguments, float);
					float_to_string(param, str);
					break;
				}
				case PARSE_LONG:
				case PARSE_LONG_LONG:
				{
					double param = va_arg(arguments, double);
					float_to_string(param, str);
					break;
				}
			}
			stream.Append(str, 32);
		}
		// text types
		else if(*s == 's')
		{
			const char* param = va_arg(arguments, const char*);
			stream.Append(param);
		}
		// size specifiers
		else if(*s == '.')
		{

		}
		else if(*s == 'h')
			sizeType = (sizeType == PARSE_SHORT) ? PARSE_BYTE : PARSE_SHORT;
		else if(*s == 'l')
			sizeType = (sizeType == PARSE_LONG) ? PARSE_LONG : PARSE_LONG_LONG;
		else if(*s == '%')
			sizeType = PARSE_NORMAL;

		++s;
	}

	va_end(arguments);

	stream += "\n";
}

const char* Log::GetText()
{
	return stream.Data();
}
