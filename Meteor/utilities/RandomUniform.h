#ifndef RANDOM_UNIFORM_H
#define RANDOM_UNIFORM_H

void reseed(unsigned long seed);
int random_int(int min, int max);
float random_float(float min, float max);
double random_double(double min, double max);
double random_uniform();

void reseed_unique();
unsigned int unique_random_int();

#endif
