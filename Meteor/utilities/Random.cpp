#include "Random.h"

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
//----------------------------------------------------------------------------------------------------

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
//----------------------------------------------------------------------------------------------------

static unsigned long mwc_random()
{
	unsigned long* x = multiplyWithCarryBuffer;

	unsigned long long sum;
	sum =	2111111111ull * (unsigned long long)(x[3])
				+ 1492ull * (unsigned long long)(x[2])
				+ 1776ull * (unsigned long long)(x[1])
				+ 5115ull * (unsigned long long)(x[0])
				+			(unsigned long long)(x[4]);

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
}

// MERSENNE TWISTER
//----------------------------------------------------------------------------------------------------
// modified version of public domain Mersenne Twister by Michael Brundage

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
	unsigned long s = b[i] & 0x80000000 | b[i2] & 0x7FFFFFFF;
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

int random_int(int min, int max)
{
	return min + int(mt_random() % (unsigned long)(max - min + 1));
}

float random_float(float min, float max)
{
	double d = double(mt_random()) / double(1 << 16 * 1 << 16);
	return min + d * (max - min);
}

// UNIQUE RANDOM NUMBER PERMUTATION
//----------------------------------------------------------------------------------------------------

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

unsigned int unique_random_int()
{
	return permute_sequence((permute_sequence(sequenceIndex++) + sequenceOffset) ^ 0x5BF03635ul);
}
