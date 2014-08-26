#include "Timer.h"

#include <time.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#elif defined(__unix__)
union posix_time_t
{
	timespec spec;
	long long value;
};
#endif

double Timer::GetTime()
{
#if defined(_WIN32)

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return double(time.QuadPart) * 1000.0 / double(frequency.QuadPart);

#elif defined(__unix__)

	posix_time_t frequency;
	clock_getres(CLOCK_MONOTONIC, &frequency.spec);
	posix_time_t time;
	clock_gettime(CLOCK_MONOTONIC, &time.spec);
	return double(time.value) * 1000.0 / double(frequency.value);

#endif
}

unsigned long Timer::GetMilliseconds()
{
	return clock() / CLOCKS_PER_SEC / 1000;
}
