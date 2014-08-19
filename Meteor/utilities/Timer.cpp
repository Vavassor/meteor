#include "Timer.h"

#include <Windows.h>

double Timer::GetTime()
{
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return double(time.QuadPart) * 1000.0 / double(frequency.QuadPart);
}