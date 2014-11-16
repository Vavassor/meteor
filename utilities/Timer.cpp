#include "Timer.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

#include <ctime>

#elif defined(__unix__)
#include <time.h>
#include <sys/time.h>
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

	timespec resolutionSpec;
	clock_getres(CLOCK_MONOTONIC, &resolutionSpec);
	timespec timeSpec;
	clock_gettime(CLOCK_MONOTONIC, &timeSpec);

	long long time = timeSpec.tv_nsec + timeSpec.tv_sec * 1e9L;
	long long resolution = resolutionSpec.tv_nsec + resolutionSpec.tv_sec * 1e9L;

	return double(time) * 1000.0 / double(resolution * 1e9L);

#endif
}

unsigned long Timer::GetMilliseconds()
{
#if defined(_WIN32)

	return clock() / CLOCKS_PER_SEC / 1000UL;

#elif defined(__unix__)

	timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000UL + time.tv_usec / 1000UL;

#endif
}
