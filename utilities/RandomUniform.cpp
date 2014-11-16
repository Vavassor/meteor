#include "RandomUniform.h"

namespace random {

#define MT_LENGTH 624
#define MT_IA 379

namespace
{
	unsigned long randomSeed = 0;

	int mersenneTwisterIndex = MT_LENGTH * sizeof(unsigned long) + 1;
	unsigned long mersenneTwisterBuffer[MT_LENGTH];

	unsigned long multiplyWithCarryBuffer[5];

	unsigned long sequenceIndex;
	unsigned long sequenceOffset;
}

// LINEAR CONGRUENTIAL GENERATOR
//-------------------------------------------------------------------------------------------------

static unsigned long lcg_seed(unsigned long seed)
{
	unsigned long previous = randomSeed;
	randomSeed = seed;
	return previous;
}

static unsigned long lcg_random()
{
	randomSeed = randomSeed * 2147001325 + 715136305;
	// shuffle non-random bits to the middle, and xor to decorrelate with seed
	return 0x31415926 ^ ((randomSeed >> 16) + (randomSeed << 16));
}

// MULTIPLY-WITH-CARRY GENERATOR
//-------------------------------------------------------------------------------------------------

static unsigned long mwc_random()
{
	unsigned long* x = multiplyWithCarryBuffer;

	unsigned long long sum;
	sum = 2111111111ull * (unsigned long long)(x[3]) +
	      1492ull       * (unsigned long long)(x[2]) +
	      1776ull       * (unsigned long long)(x[1]) +
	      5115ull       * (unsigned long long)(x[0]) +
	                      (unsigned long long)(x[4]);

	x[3] = x[2];
	x[2] = x[1];
	x[1] = x[0];
	x[4] = sum >> 32; // carry
	x[0] = sum;
	return x[0];
}

static unsigned long mwc_reseed(unsigned long seed)
{
	unsigned long s = seed;

	// initialize buffer with a few random numbers
	for(int i = 0; i < 5; ++i)
	{
		s = s * 29943829 - 1;
		multiplyWithCarryBuffer[i] = s;
	}

	// iterate a few times to increase randomness
	for(int i = 0; i < 19; ++i) mwc_random();

	return seed;
}

// MERSENNE TWISTER
//-------------------------------------------------------------------------------------------------
// modified version of Mersenne Twister by Michael Brundage

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

void reseed(unsigned long seed)
{
	lcg_seed(seed);
	for(int i = 0; i < MT_LENGTH; i++)
		mersenneTwisterBuffer[i] = lcg_random();
	mersenneTwisterIndex = 0;
}

static unsigned long mt_random()
{
	unsigned long* b = mersenneTwisterBuffer;
	int idx = mersenneTwisterIndex;

	// determine indices
	int i = idx;
    int i2 = idx + 1;
	if(i2 >= MT_LENGTH)	i2 = 0;

    int j = idx + MT_IA;
	if(j >= MT_LENGTH)	j -= MT_LENGTH;

	// Twist
	unsigned long s = (b[i] & 0x80000000) | (b[i2] & 0x7FFFFFFF);
    unsigned long r = b[j] ^ (s >> 1) ^ ((s & 1) * 0x9908B0DF);
    b[mersenneTwisterIndex] = r;
    mersenneTwisterIndex = i2;

	// Swizzle
	r ^= (r >> 11);
	r ^= (r << 7) & 0x9D2C5680;
	r ^= (r << 15) & 0xEFC60000;
	r ^= (r >> 18);

	return r;
}

int integer_range(int min, int max)
{
	return min + int(mt_random() % (unsigned long)(max - min + 1));
}

double real_range(double min, double max)
{
	double d = double(mt_random()) / 4294967296.0;
	return min + d * (max - min);
}

double uniform_real()
{
	return double(mt_random()) / 4294967296.0;
}

// UNIQUE RANDOM NUMBER PERMUTATION
//-------------------------------------------------------------------------------------------------

static unsigned long permute_sequence(unsigned long x)
{
	static const unsigned long prime = 4294967291ul;
	if(x >= prime) return x;
	unsigned long residue = ((unsigned long long) x * x) % prime;
	return (x <= prime / 2) ? residue : prime - residue;
}

void reseed_unique(unsigned long seedBase, unsigned long seedOffset)
{
	sequenceIndex = permute_sequence(permute_sequence(seedBase) + 0x682F0161ul);
	sequenceOffset = permute_sequence(permute_sequence(seedOffset) + 0x46790905ul);
}

unsigned int unique_int()
{
	return permute_sequence((permute_sequence(sequenceIndex++) + sequenceOffset) ^ 0x5BF03635ul);
}

} // namespace random
