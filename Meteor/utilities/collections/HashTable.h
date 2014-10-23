#ifndef HASH_TABLE_H
#define HASH_TABLE_H

extern inline unsigned int hash_int(unsigned int x);
extern inline unsigned int shuffle_rehash(unsigned int x);

template<typename T>
class HashTable
{
public:
	HashTable(int initialSize):
		count(0),
		deleted(0),
		hasEmpty(false),
		hasDel(false)
	{
		capacity = (initialSize >= 4) ? initialSize : 4;

		mask = initialSize - 1;

		const float LOAD_FACTOR = 0.85f;
		deleteThreshhold = (int) (initialSize * (1 - LOAD_FACTOR) / 2);
		growThreshhold = (int) (initialSize * LOAD_FACTOR);
		shrinkThreshhold = (initialSize <= 64) ? 0 : (int) (initialSize * (LOAD_FACTOR / 2.25));

		table = new Pair[initialSize];
		for(int i = 0; i < initialSize; ++i)
			table[i].key = EMPTY;
	}

	~HashTable()
	{
		delete[] table;
	}

	T* Get(unsigned int k)
	{
		if(k == EMPTY && hasEmpty) return &emptyValue;
		if(k == DEL && hasDel) return &deleteValue;

		unsigned int h = hash_int(k);
		unsigned int n = h & mask;
		
		if(table[n].key == EMPTY) return nullptr;
		if(table[n].key != DEL && table[n].key == k)
		{
			return &table[n].value;
		}

		unsigned int s = shuffle_rehash(h) | 1;

		while(true)
		{
			n = (n + s) & mask;
			if(table[n].key == EMPTY) return nullptr;
			if(table[n].key == DEL) continue;
			if(table[n].key == k) return &table[n].value;
		}

		return nullptr;
	}

	bool Add(unsigned int key, const T& value)
	{
		unsigned int h = hash_int(key);
		unsigned int n = h & mask;
		int b = -1;

		if(key == EMPTY)
		{
			if(hasEmpty)
			{
				n = hasEmpty;
				emptyValue = value;
				hasEmpty = true;
				return !n;
			}
			return false;
		}
		if(key == DEL)
		{
			if(hasDel)
			{
				n = hasDel;
				deleteValue = value;
				hasDel = true;
				return !n;
			}
			return false;
		}
		if(table[n].key != EMPTY)
		{
			if(table[n].key == DEL)
				b = n;
			else if(table[n].key == key)
				return false;

			unsigned int s = shuffle_rehash(h) | 1;

			while(true)
			{
				n = (n + s) & mask;
				if(table[n].key == EMPTY) break;
				if(table[n].key == DEL)
				{
					if(b < 0) b = n;
				}
				else if(table[n].key == key)
				{
					return false;
				}
			}
		}
		
		if(b < 0)
			b = n;
		else
			--deleted;

		table[b].key = key;
		table[b].value = value;

		++count;
		if(count > growThreshhold)
			Rehash(capacity * 2);

		return true;
	}

	bool Remove(unsigned int k)
	{
		unsigned int h = hash_int(k);
		unsigned int n = h & mask;
		
		if(k == EMPTY)
		{
			if(hasEmpty)
			{
				hasEmpty = false;
				return true;
			}
			return false;
		}
		if(k == DEL)
		{
			if(hasDel)
			{
				hasDel = false;
				return true;
			}
			return false;
		}
		if(table[n].key == EMPTY) return false;
		if(table[n].key == DEL || table[n].key != k)
		{
			unsigned int s = shuffle_rehash(h) | 1;

			while(true)
			{
				n = (n + s) & mask;
				if(table[n].key == EMPTY) return false;
				if(table[n].key == DEL) continue;
				if(table[n].key == k) break;
			}
		}

		table[n].key = DEL;
		--count;
		++deleted;

		if(count < shrinkThreshhold)
			Rehash(capacity >> 1);
		else if(deleted > deleteThreshhold)
			Rehash(capacity);

		return true;
	}

	void Clear()
	{
		delete[] table;

		hasEmpty = hasDel = false;
		count = 0;
		deleted = 0;

		table = new Pair[capacity];
		for(int i = 0; i < capacity; ++i)
			table[i].key = EMPTY;
	}

private:
	struct Pair
	{
		unsigned int key;
		T value;
	};

	static const unsigned int EMPTY = 0xFFFFFFFF;
	static const unsigned int DEL = 0xFFFFFFFE;

	Pair* table;
	unsigned int mask;
	int count, capacity;
	int deleted;

	int deleteThreshhold;
	int growThreshhold;
	int shrinkThreshhold;

	bool hasEmpty, hasDel;
	T emptyValue, deleteValue;

	void Rehash(unsigned int size)
	{
		Pair* oldTable = table;
		table = new Pair[size];
		for(int i = 0; i < capacity; ++i)
		{
			if(oldTable[i].key != EMPTY && oldTable[i].key != DEL)
				Add(oldTable[i].key, oldTable[i].value);
		}
		delete[] oldTable;
		capacity = size;
	}
};

#endif
