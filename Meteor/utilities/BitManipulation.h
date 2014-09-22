#ifndef BIT_MANIPULATION_H
#define BIT_MANIPULATION_H

#include "DataTypes.h"

int32_t get_next_power_of_two(int32_t x);
uint32_t count_set_bits(uint32_t i);
uint8_t bit_reverse8(uint8_t n);
uint32_t bit_reverse32(uint32_t n);

int64_t morton_hash(int32_t x, int32_t y, int32_t z);

#define CHECK_BIT(a, n)	((a) & 1 << (n))
#define SET_BIT(a, n)	((a) |= 1 << (n))
#define CLEAR_BIT(a, n)	((a) &= ~(1 << (n)))
#define FLIP_BIT(a, n)	((a) ^= 1 << (n))

#endif
