#include "Mutex.h"

#include <Windows.h>

Mutex::Mutex()
{
	LPCRITICAL_SECTION cs = new CRITICAL_SECTION;
    InitializeCriticalSectionAndSpinCount(cs, 500);
	criticalSection = cs;
}

Mutex::~Mutex()
{
	DeleteCriticalSection((LPCRITICAL_SECTION) criticalSection);
	delete criticalSection;
}

void Mutex::Acquire()
{
    EnterCriticalSection((LPCRITICAL_SECTION) criticalSection);
}

void Mutex::Release()
{
    LeaveCriticalSection((LPCRITICAL_SECTION) criticalSection);
}
