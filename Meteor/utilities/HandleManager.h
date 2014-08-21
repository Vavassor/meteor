#ifndef HANDLE_MANAGER_H
#define HANDLE_MANAGER_H

#include "DataTypes.h"

#include <stddef.h>

struct Handle
{
	Handle();
	Handle(uint32_t index, uint32_t counter);
 
	inline operator uint32_t() const;
 
	uint32_t index   : 16,
			 counter : 16;
};

class HandleManager
{
public:
	HandleManager(size_t capacity);
	~HandleManager();

	void Reset(size_t capacity);
	Handle Add(void* p);
	void Update(Handle handle, void* p);
	void Remove(Handle handle);
	
	void* Get(Handle handle) const;
	bool Get(Handle handle, void*& out) const;

	int GetCount() const;
	void Resize(size_t newCapacity);

private:
	struct HandleEntry
	{
		HandleEntry();
		explicit HandleEntry(uint32_t nextFreeIndex);
		
		uint32_t nextFreeIndex : 12,
				 counter	   : 15,
				 active		   : 1,
				 endOfList	   : 1;
		void*	 entry;
	};

	HandleEntry* entries;
	size_t capacity;

	int activeEntryCount;
	uint32_t firstFreeEntry;
};

#endif
