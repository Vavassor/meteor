#ifndef LOCKING_POINTER_H
#define LOCKING_POINTER_H

#include "Mutex.h"

template<typename T>
class LockingPointer
{
public:

	LockingPointer(volatile T& obj, Mutex& mtx):
		object(const_cast<T*>(&obj)),
		mutex(&mtx)
	{ mtx.Acquire(); }

	~LockingPointer() { mutex->Release(); }
   
	T& operator*() { return *object; }
	T* operator->() { return object; }

private:
	T* object;
	Mutex* mutex;
   
	LockingPointer(const LockingPointer&);
	LockingPointer& operator=(const LockingPointer&);
};

#endif
