#include "BitManipulation.h"

int32_t get_next_power_of_two(int32_t x)
{
    if(x < 0) return 0;
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
	if(x < 3) x = 3;
    return x + 1;
}

inline uint32_t count_set_bits(uint32_t i)
{
    i -= i >> 1 & 0x55555555;
    i = (i & 0x33333333) + (i >> 2 & 0x33333333);
    return (i + (i >> 4) & 0x0F0F0F0F) * 0x01010101 >> 24;
}

uint8_t bit_reverse8(uint8_t n)
{
	n = ((n & 0xAA) >> 1) + ((n & 0x55) << 1);
	n = ((n & 0xCC) >> 2) + ((n & 0x33) << 2);
	return (n >> 4) + (n << 4);
}

uint32_t bit_reverse32(uint32_t n)
{
	n = (n & 0xAAAAAAAA) >> 1 | (n & 0x55555555) << 1;
	n = (n & 0xCCCCCCCC) >> 2 | (n & 0x33333333) << 2;
	n = (n & 0xF0F0F0F0) >> 4 | (n & 0x0F0F0F0F) << 4;
	n = (n & 0xFF00FF00) >> 8 | (n & 0x00FF00FF) << 8;
	return (n >> 16) | (n << 16);
}

static inline int32_t spread(int32_t x)
{
	int32_t y = x;
	y = (y | y << 16) & 0x030000ff;
	y = (y | y << 8) & 0x0300f00f;
	y = (y | y << 4) & 0x030c30c3;
	y = (y | y << 2) & 0x09249249;
	return y;
}

int64_t morton_hash(int32_t x, int32_t y, int32_t z)
{
	return spread(x) | spread(y) << 1 | spread(z) << 2;
}
