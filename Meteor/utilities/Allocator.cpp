#include "Allocator.h"

#include <cstdlib>
#include <cstdint>

void* slab_allocator::allocate(size_t size, size_t align)
{
	return malloc(size);
}

void slab_allocator::deallocate(void* p, size_t size)
{
	free(p);
}

void* slab_allocator::reallocate(void* p, size_t size)
{
	return realloc(p, size);
}

inline unsigned char allocator::align_forward_adjustment(const void* address, unsigned char alignment)
{
	unsigned char adjustment = alignment - (uintptr_t(address) & uintptr_t(alignment - 1));
	if(adjustment == alignment)
		return 0; //already aligned
	return adjustment;
}
