#ifndef DENSE_ARRAY_H
#define DENSE_ARRAY_H

#include "HandleManager.h"
#include "Allocator.h"

#include "../Sorting.h"

template<typename T>
class DenseArray
{
private:
	HandleManager handleManager;
	
	T* data;
	size_t last;
	size_t capacity;
	Handle lastHandle;

public:
	explicit DenseArray(size_t initialCapacity):
		handleManager(initialCapacity),
		data(nullptr),
		last(0),
		capacity(initialCapacity)
	{
		data = allocator::allocate_array<T>(initialCapacity);
	}

	~DenseArray()
	{
		allocator::deallocate_array(data);
	}

	T* Get(Handle handle) const
	{
		return (T*)handleManager.Get(handle);
	}

	Handle Claim()
	{
		if(last >= capacity)
			Reserve(capacity << 1);
		new(&data[last]) T;
		lastHandle = handleManager.Add(&data[last++]);
		return lastHandle;
	}

	void Relinquish(Handle handle)
	{
		T* obj = nullptr;
		if(handleManager.Get(handle, (void*&)obj))
		{
			(*obj).~T();
			*obj = data[last];
			handleManager.Update(lastHandle, obj);
			handleManager.Remove(handle);
		}
	}

	void Reserve(size_t newCapacity)
	{
		if(newCapacity == 0) return;

		handleManager.Resize(newCapacity);

		data = allocator::reallocate_array(data, newCapacity);
		capacity = newCapacity;
	}

	void Contract()
	{
		Reserve(last + 1);
	}

	T* First() const
	{
        return data;
    }

    T* Last() const
	{
        return data + last;
    }

	template<typename IsLessFunctor>
	void Sort(const IsLessFunctor& compare)
	{
		quick_sort(data, last, compare);
	}
};

#endif
