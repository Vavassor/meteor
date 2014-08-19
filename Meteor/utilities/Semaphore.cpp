#include "Semaphore.h"

#include <Windows.h>

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
