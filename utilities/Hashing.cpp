#include "Hashing.h"

#include "BitManipulation.h"

// MULTI-BYTE HASHES
//-------------------------------------------------------------------------------------------------

uint32_t rotate_hash(const void* key, size_t len)
{
	const uint8_t* p = (const uint8_t*)key;

	uint32_t h = 0;
	for(size_t i = 0; i < len; ++i)
		h = h << 4 ^ h >> 28 ^ p[i];

	return h;
}

uint32_t djb_hash(const void* key, size_t len, uint32_t seed)
{
	const uint8_t* data = (const uint8_t*)key;

	uint32_t h = seed;
	for(size_t i = 0; i < len; ++i)
		h = 33 * h ^ data[i];

	return h;
}

uint32_t oat_hash(const void* key, size_t len)
{
	const uint8_t* p = (const uint8_t*)key;
	uint32_t h = 0;

	for(size_t i = 0; i < len; ++i)
	{
		h += p[i];
		h += h << 10;
		h ^= h >> 6;
	}

	h += h << 3;
	h ^= h >> 11;
	h += h << 15;

	return h;
}

uint32_t murmur_hash(const void* key, size_t len, uint32_t seed)
{
	uint32_t h = seed;

	const uint32_t c1 = 0xCC9E2D51;
	const uint32_t c2 = 0x1B873593;

	// body
	const uint8_t* data = (const uint8_t*)key;
	const int nblocks = len / 4;
	const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);

	for(int i = -nblocks; i != 0; ++i)
	{
		uint32_t k = blocks[i];

		k *= c1;
		k = ROTL_32(k, 15);
		k *= c2;

		h ^= k;
		h = ROTL_32(h, 13);
		h = h * 5 + 0xE6546B64;
	}

	// tail
	const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);
	uint32_t k1 = 0;

	switch(len & 3)
	{
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1:
		{
			k1 ^= tail[0];
			k1 *= c1;
			k1 = ROTL_32(k1, 15);
			k1 *= c2;
			h ^= k1;
		}
	};

	// finalization
	h ^= len;
	h ^= h >> 16;
	h *= 0x85EBCA6B;
	h ^= h >> 13;
	h *= 0xC2B2AE35;
	h ^= h >> 16;

	return h;
}

// INTEGER HASHES
//-------------------------------------------------------------------------------------------------

uint32_t jenkin_hash(uint32_t a)
{
	a = (a + 0x7ED55D16) + (a << 12);
	a = (a ^ 0xC761C23C) ^ (a >> 19);
	a = (a + 0x165667B1) + (a << 5);
	a = (a + 0xD3A2646C) ^ (a << 9);
	a = (a + 0xFD7046C5) + (a << 3);
	a = (a ^ 0xB55A4F09) ^ (a >> 16);
	return a;
}

uint32_t wang_hash_32(uint32_t k)
{
	k = (k ^ 61) ^ (k >> 16);
	k = k + (k << 3);
	k = k ^ (k >> 4);
	k = k * 0x27D4EB2D; // a prime or an odd constant
	k = k ^ (k >> 15);
	return k;
}

uint64_t wang_hash_64(uint64_t k)
{
	k = ~k + (k << 21); // k = (k << 21) - k - 1;
	k =  k ^ (k >> 24);
	k = (k + (k << 3)) + (k << 8); // k * 265
	k =  k ^ (k >> 14);
	k = (k + (k << 2)) + (k << 4); // k * 21
	k =  k ^ (k >> 28);
	k =  k + (k << 31);
	return k;
}

uint32_t mystery_hash(uint32_t a)
{
    a -= a << 6;
    a ^= a >> 17;
    a -= a << 9;
    a ^= a << 4;
    a -= a << 3;
    a ^= a << 10;
    a ^= a >> 15;
    return a;
}

// SPATIAL HASHES
//-------------------------------------------------------------------------------------------------

static inline uint32_t spread_2(uint32_t x)
{
    x &= 0x0000FFFF;
    x = (x ^ (x << 8)) & 0x00FF00FF;
    x = (x ^ (x << 4)) & 0x0F0F0F0F;
    x = (x ^ (x << 2)) & 0x33333333;
    x = (x ^ (x << 1)) & 0x55555555;
    return x;
}

uint64_t morton_hash_2d(uint32_t x, uint32_t y)
{
    return spread_2(x) | spread_2(y) << 1;
}

static inline uint32_t spread_3(uint32_t x)
{
	x = (x | x << 16) & 0x030000FF;
	x = (x | x << 8)  & 0x0300F00F;
	x = (x | x << 4)  & 0x030C30C3;
	x = (x | x << 2)  & 0x09249249;
	return x;
}

uint64_t morton_hash_3d(uint32_t x, uint32_t y, uint32_t z)
{
	return spread_3(x) | spread_3(y) << 1 | spread_3(z) << 2;
}
