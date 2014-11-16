#ifndef BIT_MANIPULATION_H
#define BIT_MANIPULATION_H

#include "DataTypes.h"

int32_t get_next_power_of_two(int32_t x);
uint32_t count_set_bits(uint32_t i);
uint8_t bit_reverse8(uint8_t n);
uint32_t bit_reverse32(uint32_t n);

// --- BASIC BIT MACROS -----------------------------------------------------------------
#define CHECK_BIT(a, n) ((a) & 1 << (n))
#define SET_BIT(a, n)   ((a) |= 1 << (n))
#define CLEAR_BIT(a, n) ((a) &= ~(1 << (n)))
#define FLIP_BIT(a, n)  ((a) ^= 1 << (n))

// --- BYTE-SWAP MACROS -----------------------------------------------------------------
#if defined(_MSC_VER)

#define BYTE_SWAP_16(x)    _byteswap_ushort(x)
#define BYTE_SWAP_32(x)    _byteswap_ulong(x)
#define BYTE_SWAP_64(x)    _byteswap_uint64(x)

#elif defined(__GNUC__)

#define BYTE_SWAP_16(x)    __builtin_bswap16(x)
#define BYTE_SWAP_32(x)    __builtin_bswap32(x)
#define BYTE_SWAP_64(x)    __builtin_bswap64(x)

#else // defined(_MSC_VER)

inline uint16_t byte_swap_16(uint16_t n)
{
	return (n & 0x00ff) << 8 | (n & 0xff00) >> 8;
}

inline uint32_t byte_swap_32(uint32_t n)
{
	return  n << 24 | (n << 8 & 0x00FF0000) | (n >> 8 & 0x0000FF00) | n >> 24;
}

inline uint64_t byte_swap_64(uint64_t n)
{
    union { 
        uint64_t ll;
        struct {
           uint32_t low, high;
        } l;
    } x;

    x.l.low = byte_swap_32(n);
    x.l.high = byte_swap_32(n >> 32);
    return x.ll;
}

#define BYTE_SWAP_16(x)    byte_swap_16(x)
#define BYTE_SWAP_32(x)    byte_swap_32(x)
#define BYTE_SWAP_64(x)    byte_swap_64(x)

#endif

// --- ROTATE MACROS --------------------------------------------------------------------
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
