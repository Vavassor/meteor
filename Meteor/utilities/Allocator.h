#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

#include <new>

#include "Alignment.h"

namespace slab_allocator
{
	void* allocate(size_t size, size_t align);
	void deallocate(void* p, size_t size);
	void* reallocate(void* p, size_t size);
}

namespace allocator
{
	extern inline unsigned char align_forward_adjustment(const void* address, unsigned char alignment);

	template<typename T>
	T* allocate_array(size_t length)
	{
		size_t headerSize = sizeof(size_t) / sizeof(T);
		if(sizeof(size_t) % sizeof(T) > 0)
			headerSize += 1;

		//Allocate extra space to store array length in the bytes before the array
		T* p = static_cast<T*>(slab_allocator::allocate(sizeof(T) * (length + headerSize), ALIGNOF(T))) + headerSize;
		*(((size_t*)p) - 1) = length;

		return p;
	}

	template<typename T>
	void deallocate_array(T* array)
	{
		//Calculate how much extra memory was allocated to store the length before the array
		size_t headerSize = sizeof(size_t) / sizeof(T);
		if(sizeof(size_t) % sizeof(T) > 0)
			headerSize += 1;

		size_t length = *(((size_t*)array) - 1);
		slab_allocator::deallocate(array - headerSize, sizeof(T) * (length + headerSize));
	}

	template<typename T>
	T* reallocate_array(T* array, size_t newSize)
	{
		//Calculate how much extra memory was allocated to store the length before the array
		size_t headerSize = sizeof(size_t) / sizeof(T);
		if(sizeof(size_t) % sizeof(T) > 0)
			headerSize += 1;

		return static_cast<T*>(slab_allocator::reallocate(array - headerSize, sizeof(T) * (newSize + headerSize)));
	}

	template<typename T>
	void** initialize_free_list(T* const mem, size_t size)
	{
		//Calculate adjustment needed to keep object correctly aligned
		unsigned char adjustment = align_forward_adjustment(mem, ALIGNOF(T));
		void** p = static_cast<unsigned char*>(mem) + adjustment;

		//Initialize free blocks list
		size_t numObjects = (size - adjustment) / sizeof(T);
		for(size_t i = 0; i < numObjects - 1; i++)
		{
			*p = static_cast<unsigned char*>(p) + sizeof(T);
			p = static_cast<void**>(*p);
		}
		*p = nullptr;
		return p;
	}
}

#endif
