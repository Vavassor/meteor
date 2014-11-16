#ifndef MUTEX_H
#define MUTEX_H

class Mutex
{
public:
    Mutex();
    ~Mutex();
    void Acquire();
    void Release();

private:
	Mutex(const Mutex&);
	void* criticalSection;
};

#endif
