#ifndef RANDOM_UNIFORM_H
#define RANDOM_UNIFORM_H

namespace random {

void reseed(unsigned long seed);
int integer_range(int min, int max);
double real_range(double min, double max);
double uniform_real();

void reseed_unique();
unsigned int unique_int();

} // namespace random

#endif
