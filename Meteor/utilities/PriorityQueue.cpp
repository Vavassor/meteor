#include "PriorityQueue.h"

#include <memory.h>

#define ELEM_ACCESS(i) reinterpret_cast<Element*>(static_cast<unsigned char*>(heap) + (i) * width)

PriorityQueue::PriorityQueue(size_t maxElements, size_t elementWidth)
{
	capacity = maxElements;
	width = sizeof(int) + elementWidth;
	heap = new unsigned char[maxElements * width];
	size = 1;
}

PriorityQueue::~PriorityQueue()
{
	delete[] heap;
}

void PriorityQueue::Add(void* data, int priority)
{
	if (size >= capacity)
	{
		capacity *= 2;
		unsigned char* temp = new unsigned char[capacity * width];
		memcpy(temp, heap, capacity * width);
		delete[] heap;
		heap = temp;
	}

	/* append at end, then up heap */
	size_t n = size++;
	for(size_t m; (m = n / 2) && priority < ELEM_ACCESS(m)->priority; n = m)
		memcpy(ELEM_ACCESS(n), ELEM_ACCESS(m), width);
	ELEM_ACCESS(n)->priority = priority;
	memcpy(&ELEM_ACCESS(n)->data, data, width - sizeof(int));
}

void* PriorityQueue::Remove(int* priority)
{
	if (size == 1) return nullptr;

	Element* element = ELEM_ACCESS(1);
	void* out = element->data;
	if(priority != nullptr)
		*priority = element->priority;
 
	// pull last item to top, then down heap.
	size--;
	size_t n = 1;
	for(size_t m; (m = n * 2) < size; n = m)
	{
		if(m + 1 < size && ELEM_ACCESS(m)->priority > ELEM_ACCESS(m + 1)->priority) m++;
		if(ELEM_ACCESS(size)->priority <= ELEM_ACCESS(m)->priority)
			break;
		memcpy(ELEM_ACCESS(n), ELEM_ACCESS(m), width);
	}
	memcpy(ELEM_ACCESS(n), ELEM_ACCESS(size), width);
	return out;
}

void* PriorityQueue::Peek(int* priority)
{
	if(size == 1) return nullptr;

	Element* element = ELEM_ACCESS(1);
	if(priority != nullptr)
		*priority = element->priority;
	return element->data;
}

void PriorityQueue::Combine(const PriorityQueue& q)
{
	if(width != q.width) return;
	Element* element = reinterpret_cast<Element*>(static_cast<unsigned char*>(q.heap) + q.width);
	for(size_t i = size - 1; i >= 1; i--, element += q.width)
		Add(element->data, element->priority);
}
