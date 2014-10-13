#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <stddef.h>

class PriorityQueue
{
private:
	struct Element
	{
		int priority;
		void* data;
	};

	void* heap;
	size_t size;
	size_t capacity;
	size_t width;

public:
	PriorityQueue(size_t maxElements, size_t elementWidth);
	~PriorityQueue();
	void Add(void* data, int priority);
	void* Remove(int* priority = nullptr);
	void* Peek(int* priority = nullptr);
	void Combine(const PriorityQueue& q);
};

#endif
