#include "Benaphore.h"

#include <windows.h>

#include "Interlocked.h"

Benaphore::Benaphore():
	counter(0)
{
    semaphore = CreateSemaphore(NULL, 0, 1, NULL);
}

Benaphore::~Benaphore()
{
    CloseHandle(semaphore);
}

void Benaphore::Lock()
{
    if(INTERLOCKED_INCREMENT(&counter) > 1)
    {
        WaitForSingleObject(semaphore, INFINITE);
    }
}

bool Benaphore::TryLock()
{
    LONG result = INTERLOCKED_COMPARE_AND_SWAP(&counter, 0, 1);
    return (result == 0);
}

void Benaphore::Unlock()
{
    if(INTERLOCKED_DECREMENT(&counter) > 0)
    {
        ReleaseSemaphore(semaphore, 1, NULL);
    }
}
