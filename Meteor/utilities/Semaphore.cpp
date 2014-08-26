#include "Semaphore.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#elif defined(__unix__)
#include <semaphore.h>
#endif

#if defined(_WIN32)

Semaphore::Semaphore(long maxCount)
{
	semaphore = CreateSemaphore(NULL, 0, maxCount, NULL);
}

Semaphore::~Semaphore()
{
	if(semaphore != NULL) CloseHandle(semaphore);
}

void Semaphore::Lock()
{
	WaitForSingleObject(semaphore, INFINITE);
}

void Semaphore::Unlock()
{
	ReleaseSemaphore(semaphore, 1, NULL);
}

#elif defined(__unix__)

Semaphore::Semaphore(long maxCount)
{
	sem_t* sem = new sem_t;
	sem_init(sem, 0, 0);
	semaphore = sem;
}

Semaphore::~Semaphore()
{
	sem_destroy((sem_t*) semaphore);
	delete semaphore;
}

void Semaphore::Lock()
{
    sem_wait((sem_t*) semaphore);
}

void Semaphore::Unlock()
{
    sem_post((sem_t*) semaphore);
}

#endif
