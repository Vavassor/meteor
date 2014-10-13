#ifndef LINKED_QUEUE_H
#define LINKED_QUEUE_H

#include "../MemoryFences.h"

// load with 'consume' (data-dependent) memory ordering
template<typename T>
T load_consume(T const* addr)
{
	T v = *const_cast<T const volatile*>(addr);
	READ_WRITE_BARRIER();
	return v;
}

// store with 'release' memory ordering
template<typename T>
void store_release(T* addr, T v)
{
	READ_WRITE_BARRIER();
	*const_cast<T volatile*>(addr) = v;
}

// cache line size on modern x86 processors (in bytes)
size_t const cache_line_size = 64;

template<typename T>
class LinkedQueue
{
public:
	LinkedQueue()
	{
		Node* n = new Node();
		n->next = 0;
		tail = head = first = tail_copy = n;
	}

	~LinkedQueue()
	{
		Node* n = first;
		do
		{
			Node* next = n->next;
			delete n;
			n = next;
		}
		while (n);
	}

	void Enqueue(T v)
	{
		Node* n = AllocNode();
		n->next = 0;
		n->value = v;
		store_release(&head->next, n);
		head = n;
	}

	bool Dequeue(T& v)
	{
		if (load_consume(&tail->next))
		{
			v = tail->next->value;
			store_release(&tail, tail->next);
			return true;
		}
		else
		{
			return false;
		}
	}

private:
	struct Node
	{
		Node* next;
		T value;
	};

	// consumer variables
	// accessed only by consumer
	Node* tail;

	// delimiter between consumer part and producer part,
	// so that they situated on different cache lines
	char cache_line_pad [cache_line_size];

	// producer variables
	// accessed only by producer
	Node* head;
	Node* first; // last unused node (tail of node cache)
	Node* tail_copy; // helper (points somewhere between first and tail)

	Node* AllocNode()
	{
		// first tries to allocate node from internal node cache,
		// if attempt fails, allocates node via ::operator new()
		if(first != tail_copy)
		{
			Node* n = first;
			first = first->next;
			return n;
		}
		tail_copy = load_consume(&tail);
		if(first != tail_copy)
		{
			Node* n = first;
			first = first->next;
			return n;
		}
		Node* n = new Node;
		return n;
	}

	LinkedQueue(LinkedQueue const&);
	LinkedQueue& operator = (LinkedQueue const&);
};

#endif
