#ifndef SEMAPHORE_H
#define SEMAPHORE_H

class Semaphore
{
public:
	void* semaphore;

	Semaphore(long maxCount = 1);
	~Semaphore();
	void Lock();
	void Unlock();
};

#endif
