#include "Logging.h"

#include "String.h"
#include "FileHandling.h"
#include "Conversion.h"

#if defined(_MSC_VER) && defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#else
#include <cstdio>
#endif

#include <time.h>
#include <stdarg.h>

#define LOG_FILE_NAME "log_file.txt"

namespace Log
{
	String stream;
	long ticks;

	enum ParseSizeParameter
	{
		PARSE_MEDIAN,
		PARSE_BYTE,
		PARSE_SHORT,
		PARSE_LONG,
		PARSE_LONG_LONG,
	};

	enum ParseMode
	{
		MODE_APPEND,
		MODE_INSERT,
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

void Log::Output(bool printToConsole)
{
	if(stream.Size() == 0) return;

	#if defined(_DEBUG)
	if(printToConsole)
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
		case Log::ISSUE: return "ERROR";
		case Log::INFO:  return "INFO";
		case Log::DEBUG: return "DEBUG";
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
	stream.Reserve(stream.Size() + 80);
	stream.Append("LOG-");

	time_t signature = time(nullptr);
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

	stream.Append(log_level_name(level));
	stream += ": ";

	// write parameter data to log
	va_list arguments;
	va_start(arguments, format);

	ParseMode mode = MODE_APPEND;
	ParseSizeParameter sizeType = PARSE_MEDIAN;

	char* s = (char*) format;
	char* f = s;
	while(*s)
	{
		switch(mode)
		{
			case MODE_APPEND:
			{
				// go through string until '%' is found, which
				// signifies a sequence to be replaced
				if(*s == '%')
				{
					if(s != f)
					{
						stream.Append(f, s);
					}
					mode = MODE_INSERT;
					sizeType = PARSE_MEDIAN;
				}
				break;
			}

			case MODE_INSERT:
			{
				// determine type of argument in parameter list and replace
				// the '%' sub-sequence with the parsed string as directed

				// size specifiers
				if(*s == '.')
				{

				}
				else if(*s == 'h')
				{
					sizeType = (sizeType == PARSE_SHORT) ? PARSE_BYTE : PARSE_SHORT;
					break;
				}
				else if(*s == 'l')
				{
					sizeType = (sizeType == PARSE_LONG) ? PARSE_LONG : PARSE_LONG_LONG;
					break;
				}

				// number types
				if(*s == 'i')
				{
					char str[20];
					switch(sizeType)
					{
						LOG_CASE(PARSE_BYTE, char, int_to_string)
						LOG_CASE(PARSE_SHORT, short, int_to_string)
						LOG_CASE(PARSE_MEDIAN, int, int_to_string)
						LOG_CASE(PARSE_LONG, long, int_to_string)
						LOG_CASE(PARSE_LONG_LONG, long long, int_to_string)
					}
					stream.Append(str);
				}
				else if(*s == 'u')
				{
					char str[20];
					switch(sizeType)
					{
						LOG_CASE(PARSE_BYTE, unsigned char, int_to_string)
						LOG_CASE(PARSE_SHORT, unsigned short, int_to_string)
						LOG_CASE(PARSE_MEDIAN, unsigned int, int_to_string)
						LOG_CASE(PARSE_LONG, unsigned long, int_to_string)
						LOG_CASE(PARSE_LONG_LONG, unsigned long long, int_to_string)
					}
					stream.Append(str);
				}
				else if(*s == 'x')
				{
					char str[20];
					unsigned long long param = va_arg(arguments, unsigned long long);
					int_to_string(param, str, 16);
					stream.Append(str);
				}
				else if(*s == 'f')
				{
					char str[32];
					switch(sizeType)
					{
						case PARSE_BYTE:
						case PARSE_SHORT:
						case PARSE_MEDIAN:
						case PARSE_LONG:
						case PARSE_LONG_LONG:
						{
							double param = va_arg(arguments, double);
							float_to_string(param, str);
							break;
						}
					}
					stream.Append(str);
				}
				// text types
				else if(*s == 's')
				{
					const char* param = va_arg(arguments, const char*);
					stream.Append(param);
				}

				f = s + 1;
				mode = MODE_APPEND;

				break;
			}
		}

		++s;
	}

	va_end(arguments);

	if(s != f)
	{
		stream.Append(f, s);
	}

	stream.Append("\n");
}

const char* Log::Get_Text()
{
	return stream.Data();
}
