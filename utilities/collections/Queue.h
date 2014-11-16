#ifndef QUEUE_H
#define QUEUE_H

#include <new>
#include <stddef.h>

template<typename T>
class Queue
{
private:
	T* buffer;
	size_t capacity;
	size_t front, rear;
	
public:
	explicit Queue(size_t capacity):
		capacity(capacity),
		front(0),
		rear(0)
	{
		buffer = (T*) ::operator new[](sizeof(T) * capacity);
	}

	~Queue()
	{
		::operator delete[](buffer);
	}

	void Enqueue(const T& element)
	{
		size_t next = (rear + 1) % capacity;
		if(next == front) return;
		rear = next;
		new(&buffer[next]) T(element);
	}

	bool Dequeue(T* element)
	{
		if(front == rear) return false;
		front = (front + 1) % capacity;
		*element = buffer[front];
		buffer[front].~T();
		return true;
	}

	inline T& Front()
	{
		return buffer[front];
	}

	inline const T& Front() const
	{
		return buffer[front];
	}

	inline bool Is_Empty() const
	{
		return front == rear;
	}

	void Clear()
	{
		for(auto it = First(), end = Last(); it != end; ++it)
			it->~T();

		front = rear = 0;
		::operator delete[](buffer);
		buffer = ::operator new[](sizeof(T) * capacity);
	}

	Iterator First()
	{
		return Iterator(*this, buffer + front);
	}

	ConstIterator First() const
	{
		return ConstIterator(*this, buffer + front);
	}

	Iterator Last()
	{
		return Iterator(*this, buffer + rear);
	}

	ConstIterator Last() const
	{
		return ConstIterator(*this, buffer + rear);
	}

	friend class Iterator;
	class Iterator
	{
	private:
		Queue& container;
		T* current;

	public:
		Iterator(Queue& queue, T* pointer):
			container(queue),
			current(pointer)
		{}

		Iterator(const Iterator& other):
			container(other.container),
			current(other.current)
		{}

		T& operator * () { return *current; }
		T* operator -> () { return current; }
		bool operator != (const Iterator& other) const { return current != other.current; }
		bool operator == (const Iterator& other) const { return current == other.current; }

		Iterator& operator++()
		{
			T* base = container.buffer;
			current = base + ((current - base + 1) % container.capacity);
			return *this;
		}
	};

	friend class ConstIterator;
	class ConstIterator
	{
	private:
		Queue& container;
		T* current;

	public:
		ConstIterator(Queue& queue, T* pointer):
			container(queue),
			current(pointer)
		{}

		ConstIterator(const ConstIterator& other):
			container(other.container),
			current(other.current)
		{}

		const T& operator * () const { return *current; }
		const T* operator -> () const { return current; }
		bool operator != (const Iterator& other) const { return current != other.current; }
		bool operator == (const Iterator& other) const { return current == other.current; }

		ConstIterator& operator++()
		{
			T* base = container.buffer;
			current = base + ((current - base + 1) % container.capacity);
			return *this;
		}
	};
};

#endif
