#ifndef INTEGER_HASH_TABLE_H
#define INTEGER_HASH_TABLE_H

#include <stddef.h>

class IntegerHashTable
{
private:
	struct Cell
    {
        size_t key;
        size_t value;
    };

    Cell* cells;
    size_t capacity;
    size_t population;
    bool isZeroUsed;
    Cell zeroCell;
    
	Cell* LookupCell(size_t key);
	Cell* FindUnusedCell(size_t key);
	void Delete(Cell* cell);
    void Repopulate(size_t desiredSize);

public:
    IntegerHashTable(size_t initialSize = 8);
    ~IntegerHashTable();

    size_t Lookup(size_t key);
    void Insert(size_t key, size_t value);
	void Delete(size_t key);
    
    void Clear();
    void Compact();

    //----------------------------------------------
    //  Iterator
    //----------------------------------------------
    friend class Iterator;
    class Iterator
    {
    private:
        IntegerHashTable& table;
        Cell* current;

    public:
        Iterator(IntegerHashTable &table);
        Cell* Next();
        inline Cell* operator*() const { return current; }
        inline Cell* operator->() const { return current; }
    };
};

#endif
