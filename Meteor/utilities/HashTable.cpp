#include "HashTable.h"

inline unsigned int hash_int(unsigned int x)
{
   x ^= x << 3;
   x += x >> 5;
   x ^= x << 4;
   x += x >> 17;
   x ^= x << 25;
   x += x >> 6;
   return x;
}

static inline unsigned int shift_rehash(unsigned int x)
{
	return x + (x >> 6) + (x >> 19);
}

inline unsigned int shuffle_rehash(unsigned int x)
{
   x = shift_rehash(x);
   x += x << 16;

   // pearson's shuffle
   x ^= x << 3;
   x += x >> 5;
   x ^= x << 2;
   x += x >> 15;
   x ^= x << 10;

   return shift_rehash(x);
}
