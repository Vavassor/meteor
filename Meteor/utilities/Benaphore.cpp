#include "Benaphore.h"

#include "Interlocked.h"

#if defined(_WIN32)
#include <windows.h>
#elif defined(__unix__)
#include <semaphore.h>
#endif

#if defined(_WIN32)

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

#elif defined(__unix__)

Benaphore::Benaphore():
	counter(0)
{
	sem_t* sem = new sem_t;
	sem_init(sem, 0, 0);
	semaphore = sem;
}

Benaphore::~Benaphore()
{
	sem_destroy((sem_t*) semaphore);
	delete semaphore;
}

void Benaphore::Lock()
{
    if(INTERLOCKED_INCREMENT(&counter) > 1)
    {
    	sem_wait((sem_t*) semaphore);
    }
}

bool Benaphore::TryLock()
{
    int result = INTERLOCKED_COMPARE_AND_SWAP(&counter, 0, 1);
    return (result == 0);
}

void Benaphore::Unlock()
{
    if(INTERLOCKED_DECREMENT(&counter) > 0)
    {
    	sem_post((sem_t*) semaphore);
    }
}

#endif
