#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "../Hashing.h"

#include <cstring>
#include <stddef.h>
#include <new>

template<typename T>
class HashMap
{
public:
	explicit HashMap(size_t initialCapacity):
		capacity(initialCapacity),
		count(0)
	{
		values = (T*) ::operator new[](sizeof(T) * initialCapacity);
		keys = new char*[initialCapacity];
		memset(keys, 0, sizeof(char*) * initialCapacity);
	}

	~HashMap()
	{
		for(size_t i = 0; i < capacity; ++i)
		{
			if(keys[i] == nullptr) continue;
			values[i].~T();
		}
		::operator delete[](values);

		delete[] keys;
	}

	void Add(const char* key, const T& value)
	{
		size_t index = Hash_Key(key);
		if(index >= capacity)
		{
			Rehash();
			index = Hash_Key(key);
		}

		new(&values[index]) T(value);
		keys[index] = (char*) key;
		++count;
	}

	T* Get(const char* key)
	{
		// find key
		uint32_t curr = Hash_String(key);

		// linear probing, if necessary
		for(int i = 0; i < MAX_CHAIN_LENGTH; ++i)
		{
			if(keys[curr] != nullptr && strcmp(keys[curr], key) == 0)
				return &values[curr];

			curr = (curr + 1) % capacity;
		}

		return nullptr;
	}

	bool Remove(const char* key)
	{
		// find key
		uint32_t curr = Hash_String(key);

		// linear probing, if necessary
		for(int i = 0; i < MAX_CHAIN_LENGTH; ++i)
		{
			if(keys[curr] != nullptr && strcmp(keys[curr], key) == 0)
			{
				// empty the bucket
				keys[curr] = nullptr;
				values[curr].~T();

				count--;
				return true;
			}
			curr = (curr + 1) % capacity;
		}

		return false;
	}

private:
	static const int MAX_CHAIN_LENGTH = 8;

	char** keys;
	T* values;
	size_t capacity;
	size_t count;

	uint32_t Hash_String(const char* s)
	{
		return murmur_hash(s, strlen(s), 0) % capacity;
	}

	size_t Hash_Key(const char* key)
	{
		// if full, return immediately
		if(count >= capacity / 2) return capacity;

		// find the best index
		uint32_t curr = Hash_String(key);

		// linear probing
		for(int i = 0; i < MAX_CHAIN_LENGTH; ++i)
		{
			if(keys[curr] == nullptr)
				return curr;

			if(keys[curr] != nullptr && strcmp(keys[curr], key) == 0)
				return curr;

			curr = (curr + 1) % capacity;
		}

		return capacity;
	}

	void Rehash()
	{
		// double the capacity
		size_t old_size = capacity;
		capacity = 2 * capacity;

		// save current keys/values and reallocate
		T* old_values = values;
		values = (T*) ::operator new[](sizeof(T) * capacity);

		char** old_keys = keys;
		keys = new char*[capacity];
		memset(keys, 0, sizeof(char*) * capacity);

		// Rehash the elements
		count = 0;
		for(size_t i = 0; i < old_size; ++i)
		{
			if(old_keys[i] == nullptr) continue;

			Add(old_keys[i], old_values[i]);
		}

		// destruct and delete old pairs
		for(size_t i = 0; i < old_size; ++i)
		{
			if(old_keys[i] == nullptr) continue;
			old_values[i].~T();
		}
		::operator delete[](old_values);

		delete[] old_keys;
	}
};

#endif
