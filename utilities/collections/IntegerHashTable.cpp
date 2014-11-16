#include "IntegerHashTable.h"

#include "../BitManipulation.h"

#include <cstring>

static inline uint32_t murmur_hash_32(uint32_t h)
{
	h ^= h >> 16;
	h *= 0x85EBCA6B;
	h ^= h >> 13;
	h *= 0xC2B2AE35;
	h ^= h >> 16;
	return h;
}

static inline uint64_t murmur_hash(uint64_t k)
{
	k ^= k >> 33;
	k *= 0xFF51AFD7ED558CCD;
	k ^= k >> 33;
	k *= 0xC4CEB9FE1A85EC53;
	k ^= k >> 33;
	return k;
}

#define FIRST_CELL(hash) (cells + ((hash) & (capacity - 1)))
#define CIRCULAR_NEXT(c) ((c) + 1 != cells + capacity ? (c) + 1 : cells)
#define CIRCULAR_OFFSET(a, b) ((b) >= (a) ? (b) - (a) : capacity + (b) - (a))

IntegerHashTable::IntegerHashTable(size_t initialSize)
{
    capacity = get_next_power_of_two(initialSize);
    cells = new Cell[capacity];
    memset(cells, 0, sizeof(Cell) * capacity);
    population = 0;

    // Initialize zero cell
    isZeroUsed = false;
    zeroCell.key = 0;
    zeroCell.value = 0;
}

IntegerHashTable::~IntegerHashTable()
{
    delete[] cells;
}

size_t IntegerHashTable::Lookup(size_t key)
{
	Cell* cell = LookupCell(key);
	if(cell != nullptr) return cell->value;
	else return 0;
}

void IntegerHashTable::Insert(size_t key, size_t value)
{
	Cell* cell = FindUnusedCell(key);
	if(cell != nullptr)
	{
		cell->key = key;
		cell->value = value;
	}
}

void IntegerHashTable::Delete(size_t key)
{
    Cell* cell = LookupCell(key);
    if(cell != nullptr) Delete(cell);
}

IntegerHashTable::Cell* IntegerHashTable::LookupCell(size_t key)
{
    if(key)
    {
        // Check regular cells
        for(Cell* cell = FIRST_CELL(murmur_hash(key));; cell = CIRCULAR_NEXT(cell))
        {
            if(cell->key == key) return cell;
            if(!cell->key) return nullptr;
        }
    }
    else
    {
        // Check zero cell
        if(isZeroUsed) return &zeroCell;
        return nullptr;
    }
    return nullptr;
}

IntegerHashTable::Cell* IntegerHashTable::FindUnusedCell(size_t key)
{
    if(key)
    {
        // Check regular cells
        while(true)
        {
            for(Cell* cell = FIRST_CELL(murmur_hash(key));; cell = CIRCULAR_NEXT(cell))
            {
                if(cell->key == key) return cell;
                if(cell->key == 0)
                {
                    // Insert here
                    if((population + 1) * 4 >= capacity * 3)
                    {
                        Repopulate(capacity * 2);
                        break;
                    }
                    ++population;
                    return cell;
                }
            }
        }
    }
    else
    {
        // Check zero cell
        if(!isZeroUsed)
        {
            // Insert here
            isZeroUsed = true;
            if(++population * 4 >= capacity * 3)
			{
				// Even though we didn't use a regular slot, let's keep the sizing rules consistent
                Repopulate(capacity * 2);
			}
        }
        return &zeroCell;
    }

    return nullptr;
}

void IntegerHashTable::Delete(Cell* cell)
{
    if(cell != &zeroCell)
    {
        // Remove this cell by shuffling neighboring cells so there are no gaps in anyone's probe chain
        for(Cell* neighbor = CIRCULAR_NEXT(cell);; neighbor = CIRCULAR_NEXT(neighbor))
        {
            if(!neighbor->key)
            {
                // There's nobody to swap with. Go ahead and clear this cell, then return
                cell->key = 0;
                cell->value = 0;
                population--;
                return;
            }
            Cell* ideal = FIRST_CELL(murmur_hash(neighbor->key));
            if(CIRCULAR_OFFSET(ideal, cell) < CIRCULAR_OFFSET(ideal, neighbor))
            {
                // Swap with neighbor, then make neighbor the new cell to remove.
                *cell = *neighbor;
                cell = neighbor;
            }
        }
    }
    else
    {
        // Delete zero cell
        isZeroUsed = false;
        cell->value = 0;
        population--;
    }
}

// Does not resize the array
void IntegerHashTable::Clear()
{
    // Clear regular cells
    memset(cells, 0, sizeof(Cell) * capacity);
    population = 0;

    // Clear zero cell
    isZeroUsed = false;
    zeroCell.value = 0;
}

void IntegerHashTable::Compact()
{
    Repopulate(get_next_power_of_two((population * 4 + 3) / 3));
}

void IntegerHashTable::Repopulate(size_t desiredSize)
{
    // Get start/end pointers of old array
    Cell* oldCells = cells;
    Cell* end = cells + capacity;

    // Allocate new array
    capacity = desiredSize;
    cells = new Cell[capacity];
    memset(cells, 0, sizeof(Cell) * capacity);

    // Iterate through old array
    for(Cell* c = oldCells; c != end; c++)
    {
        if(c->key)
        {
            // Insert this element into new array
            for(Cell* cell = FIRST_CELL(murmur_hash(c->key));; cell = CIRCULAR_NEXT(cell))
            {
                if(!cell->key)
                {
                    *cell = *c;
                    break;
                }
            }
        }
    }
    delete[] oldCells;
}

IntegerHashTable::Iterator::Iterator(IntegerHashTable &pTable) : table(pTable)
{
    current = &table.zeroCell;
    if(!table.isZeroUsed) Next();
}

IntegerHashTable::Cell* IntegerHashTable::Iterator::Next()
{
    if(current == nullptr) return current;

    // Iterate past zero cell
    if(current == &table.zeroCell)
        current = &table.cells[-1];

    // Iterate through the regular cells
    Cell* end = table.cells + table.capacity;
    while(++current != end)
    {
        if(current->key) return current;
    }
    return current = nullptr;
}
