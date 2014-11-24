#include "HandleManager.h"

Handle::Handle():
	index(0),
	counter(1)
{}

Handle::Handle(uint32_t index, uint32_t counter):
	index(index),
	counter(counter)
{}

Handle::operator uint32_t() const
{
	return counter << 16 | index;
}

HandleManager::HandleEntry::HandleEntry():
	nextFreeIndex(0),
	counter(1),
	active(0),
	endOfList(0),
	entry(nullptr)
{}

HandleManager::HandleEntry::HandleEntry(uint32_t nextFreeIndex):
	nextFreeIndex(nextFreeIndex),
	counter(1),
	active(0),
	endOfList(0),
	entry(nullptr)
{}

HandleManager::HandleManager(size_t capacity):
	entries(nullptr),
	capacity(capacity),
	activeEntryCount(0),
	firstFreeEntry(0)
{
	Reset(capacity);
}

HandleManager::~HandleManager()
{
	delete[] entries;
}

void HandleManager::Reset(size_t capacity)
{
	activeEntryCount = 0;
	firstFreeEntry = 0;

	delete[] entries;
	entries = new HandleEntry[capacity];
	for(int i = 0; i < capacity - 1; i++)
		entries[i] = HandleEntry(i + 1);
	entries[capacity - 1] = HandleEntry();
	entries[capacity - 1].endOfList = true;
}

Handle HandleManager::Add(void* p)
{
	const int newIndex = firstFreeEntry;
	firstFreeEntry = entries[newIndex].nextFreeIndex;

	entries[newIndex].nextFreeIndex = 0;
	entries[newIndex].counter++;
	if(entries[newIndex].counter == 0)
		entries[newIndex].counter = 1;
	entries[newIndex].active = true;
	entries[newIndex].entry = p;

	activeEntryCount++;

	return Handle(newIndex, entries[newIndex].counter);
}

void HandleManager::Update(Handle handle, void* p)
{
	entries[handle.index].entry = p;
}

void HandleManager::Remove(const Handle handle)
{
	const uint32_t index = handle.index;
	entries[index].nextFreeIndex = firstFreeEntry;
	entries[index].active = 0;
	firstFreeEntry = index;

	activeEntryCount--;
}

void* HandleManager::Get(Handle handle) const
{
	void* p = nullptr;
	if(!Get(handle, p)) return nullptr;
	return p;
}

bool HandleManager::Get(const Handle handle, void*& out) const
{
	const int index = handle.index;
	if (entries[index].counter != handle.counter ||
	    entries[index].active == false)
		return false;

	out = entries[index].entry;
	return true;
}

int HandleManager::GetCount() const
{
	return activeEntryCount;
}

void HandleManager::Resize(size_t newCapacity)
{
	if(newCapacity < 1) return;

	HandleEntry* temp = new HandleEntry[newCapacity];
	for(size_t i = 0; i < capacity - 1; i++)
		temp[i] = entries[i];
	delete[] entries;
	capacity = newCapacity;
	entries = temp;
}
