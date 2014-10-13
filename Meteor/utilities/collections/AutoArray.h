#ifndef AUTO_ARRAY_H
#define AUTO_ARRAY_H

#include <stddef.h>
#include <new>

template<typename T>
class AutoArray
{
private:
	T* buffer;
	T* last;
	T* end;

	inline size_t Capacity()
	{
		return end - buffer;
	}

public:
	explicit AutoArray(size_t initialCapacity = 10)
	{
		buffer = (T*) ::operator new(initialCapacity * sizeof(T));
		last = buffer;
		end = buffer + initialCapacity;
	}

	~AutoArray()
	{
		for(T *it = buffer, *stop = last; it != stop; ++it)
			it->~T();
		::operator delete(buffer);
	}

	void Push(const T& element)
	{
		if(last >= end)
			Resize(Capacity() << 1);
		new(last++) T(element);
	}

	bool Pop(T* element)
	{
		if(last == buffer) return false;
		*element = *--last;
		last->~T();
		return true;
	}

	bool Set(size_t index, const T& element)
	{
		if(index >= Count())
			return false;
		buffer[index] = element;
		return true;
	}

	void Clear()
	{
		for(T *it = buffer, *stop = last; it != stop; ++it)
			it->~T();
		last = buffer;
	}

	void Resize(size_t newSize)
	{
		if(newSize == 0) return;

		T* newArray = (T*) ::operator new(newSize * sizeof(T));

		size_t count = Count();
		size_t minSize = (newSize < count) ? newSize : count;
		for(size_t i = 0; i < minSize; ++i)
			new(&newArray[i]) T(buffer[i]);

		::operator delete(buffer);
		buffer = newArray;

		// reallocation invalidated these, so reset them
		last = buffer + minSize;
		end = buffer + newSize;
	}

	void Contract()
	{
		Resize(Count());
	}

	T* Get(size_t index) const
	{
		if(index >= Count())
			return nullptr;
		return buffer[index];
	}

	T& operator[](size_t index)
	{
		return buffer[index];
	}

	const T& operator[](size_t index) const
	{
		return buffer[index];
	}

	inline T* First() const
	{
		return buffer;
	}

	inline T* Last() const
	{
		return last;
	}

	inline size_t Count() const
	{
		return last - buffer;
	}
};

#endif
