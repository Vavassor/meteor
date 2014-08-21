#include "Mutex.h"

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__unix__)
#include <pthread.h>
#endif

#if defined(_WIN32)

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

#elif defined(__unix__)

Mutex::Mutex()
{
	pthread_mutex_t* mutex = new pthread_mutex_t;
	pthread_mutex_init(mutex, nullptr);
	criticalSection = mutex;
}

Mutex::~Mutex()
{
	pthread_mutex_destroy((pthread_mutex_t*) criticalSection);
	delete criticalSection;
}

void Mutex::Acquire()
{
	pthread_mutex_lock((pthread_mutex_t*) criticalSection);
}

void Mutex::Release()
{
	pthread_mutex_unlock((pthread_mutex_t*) criticalSection);
}

#endif
