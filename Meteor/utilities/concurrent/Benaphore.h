#ifndef BENAPHORE_H
#define BENAPHORE_H

class Benaphore
{
private:
    long counter;
    void* semaphore;

public:
    Benaphore();
    ~Benaphore();
    void Lock();
	bool TryLock();
    void Unlock();
};

#endif
