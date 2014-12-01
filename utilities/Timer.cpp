#include "Timer.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

#include <ctime>

#if defined(_MSC_VER)
#include <intrin.h>
#pragma intrinsic(__rdtsc)

#elif defined(__GNUC__)
#include <cstdint>
#endif

#elif defined(__unix__)
#include <time.h>
#include <sys/time.h>
#endif // defined(_WIN32)

namespace
{
	double performance_counter_resolution;
}

void Timer::Initialize()
{
#if defined(_WIN32)

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	performance_counter_resolution = static_cast<double>(frequency.QuadPart) / 1000.0;

#elif defined(__unix__)

	timespec resolutionSpec;
	clock_getres(CLOCK_MONOTONIC_RAW, &resolutionSpec);
	long long resolution = resolutionSpec.tv_nsec + resolutionSpec.tv_sec * 1e9L;
	performance_counter_resolution = static_cast<double>(resolution * 1e9L) / 1000.0;

#endif
}

double Timer::Get_Time()
{
#if defined(_WIN32)

	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	return static_cast<double>(time.QuadPart) / performance_counter_resolution;

#elif defined(__unix__)

	timespec timeSpec;
	clock_gettime(CLOCK_MONOTONIC_RAW, &timeSpec);
	long long time = timeSpec.tv_nsec + timeSpec.tv_sec * 1e9L;
	return static_cast<double>(time) / performance_counter_resolution;

#endif
}

unsigned long Timer::Get_Milliseconds()
{
#if defined(_WIN32)

	return clock() / CLOCKS_PER_SEC / 1000UL;

#elif defined(__unix__)

	timeval time;
	gettimeofday(&time, NULL);
	return time.tv_sec * 1000UL + time.tv_usec / 1000UL;

#endif
}

unsigned long long Timer::Cycle_Count()
{
#if defined(_MSC_VER)

	return __rdtsc();

#elif defined(__GNU_C__)

	uint32_t lower, upper;
	__asm__ __volatile__("rdtsc": "=a"(lower), "=d"(upper));
	return static_cast<uint64_t>(upper) << 32 | static_cast<uint64_t>(lower);

#endif
}