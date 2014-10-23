#ifndef BIT_MANIPULATION_H
#define BIT_MANIPULATION_H

#include "DataTypes.h"

int32_t get_next_power_of_two(int32_t x);
uint32_t count_set_bits(uint32_t i);
uint8_t bit_reverse8(uint8_t n);
uint32_t bit_reverse32(uint32_t n);
uint32_t byte_swap(uint32_t n);

// --- BASIC BIT MACROS -------------------------
#define CHECK_BIT(a, n) ((a) & 1 << (n))
#define SET_BIT(a, n)   ((a) |= 1 << (n))
#define CLEAR_BIT(a, n) ((a) &= ~(1 << (n)))
#define FLIP_BIT(a, n)  ((a) ^= 1 << (n))

// --- ROTATE MACROS ----------------------------
#if defined(_MSC_VER)

#include <stdlib.h>

#define ROTL_32(x,y)    _rotl(x,y)
#define ROTL_64(x,y)    _rotl64(x,y)

#define ROTR_32(x,r)    _rotr(x,r)
#define ROTR_64(x,r)    _rotr64(x,r)

#else

inline uint32_t rotl_32(uint32_t x, int8_t r)
{
	return x << r | x >> (32 - r);
}

inline uint64_t rotl_64(uint64_t x, int8_t r)
{
	return x << r | x >> (64 - r);
}

inline uint32_t rotr_32(uint32_t x, int8_t r)
{
	return x >> r | x << (32 - r);
}

inline uint64_t rotr_64(uint64_t x, int8_t r)
{
	return x >> r | x << (64 - r);
}

#define ROTL_32(x,y)    rotl_32(x,y)
#define ROTL_64(x,y)    rotl_64(x,y)

#define ROTR_32(x,r)    rotr_32(x,r)
#define ROTR_64(x,r)    rotr_64(x,r)

#endif // defined(_MSC_VER)

#endif
