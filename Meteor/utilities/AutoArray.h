#ifndef AUTO_ARRAY_H
#define AUTO_ARRAY_H

#include <stddef.h>

template<typename T>
class AutoArray
{
private:
	T* buffer;
	size_t last;
	size_t capacity;

public:
	explicit AutoArray(size_t initialCapacity):
		last(0),
		capacity(initialCapacity)
	{
		buffer = new T[initialCapacity];
	}

	~AutoArray()
	{
		delete[] buffer;
	}

	void Push(const T& element)
	{
		if(last + 1 >= capacity)
			Resize(capacity << 1);
		buffer[++last] = element;
	}

	bool Pop(T* element)
	{
		if(last == 0) return false;
		*element = buffer[last--];
		return true;
	}

	void Set(int index, const T& element)
	{
		if(index < 0) return;
		if(index > last)
			Resize(index + 1);
		buffer[index] = element;
	}

	void Clear()
	{
		delete[] buffer;
		buffer = new T[capacity];
		last = 0;
	}

	void Resize(size_t newSize)
	{
		if(newSize == 0) return;

		T* newArray = new T[newSize];
		size_t minSize = (newSize < capacity) ? newSize : capacity;
		for(size_t i = 0; i < minSize; i++)
			newArray[i] = buffer[i];
		delete[] buffer;
		buffer = newArray;
		capacity = newSize;

		if(last >= minSize)
			last = minSize - 1;
	}

	void Contract()
	{
		Resize(last + 1);
	}

	T* Get(int index) const
	{
		if(index < 0 || index > last)
			return nullptr;
		return buffer[index];
	}

	T& operator[](int index)
	{
		return buffer[index];
	}

	const T& operator[](int index) const
	{
		return buffer[index];
	}

	inline T* First() const
	{
		return buffer;
	}

	inline T* Last() const
	{
		return buffer + last;
	}

	inline size_t Count() const
	{
		return last + 1;
	}
};

#endif
