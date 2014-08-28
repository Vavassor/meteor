#ifndef QUEUE_H
#define QUEUE_H

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
		buffer = new T[capacity];
	}

	~Queue()
	{
		delete[] buffer;
	}

	void Enqueue(const T& element)
	{
		size_t next = (rear + 1) % capacity;
		if(next == front) return;
		rear = next;
		buffer[next] = element;
	}

	bool Dequeue(T* element)
	{
		if(front == rear) return false;
		front = (front + 1) % capacity;
		*element = buffer[front];
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

	inline bool IsEmpty() const
	{
		return front == rear;
	}

	void Clear()
	{
		front = rear = 0;
		delete[] buffer;
		buffer = new T[capacity];
	}

	Iterator First()
	{
		return Iterator(*this, buffer + front);
	}

	ConstIterator First()
	{
		return ConstIterator(*this, buffer + front);
	}

	Iterator Last()
	{
		return Iterator(*this, buffer + rear);
	}

	ConstIterator Last()
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
