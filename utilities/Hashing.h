#ifndef HASHING_H
#define HASHING_H

#include "DataTypes.h"

#include <stddef.h>

//--- MULTI-BYTE HASHES ---------------------------------------------------------------------------

uint32_t rotate_hash(const void* key, size_t len);
uint32_t djb_hash(const void* key, size_t len, uint32_t seed);
uint32_t oat_hash(const void* key, size_t len);
uint32_t murmur_hash(const void* key, size_t len, uint32_t seed);

//--- INTEGER HASHES ------------------------------------------------------------------------------

uint32_t jenkin_hash(uint32_t a);
uint32_t wang_hash_32(uint32_t k);
uint64_t wang_hash_64(uint64_t k);
uint32_t mystery_hash(uint32_t a);

//--- SPATIAL HASHES ------------------------------------------------------------------------------

uint64_t morton_hash_2d(uint32_t x, uint32_t y);
uint64_t morton_hash_3d(uint32_t x, uint32_t y, int32_t z);

#endif
