#ifndef RANDOM_H
#define RANDOM_H

void reseed(unsigned long seed);
int random_int(int min, int max);
float random_float(float min, float max);

void reseed_unique();
unsigned int unique_random_int();

#endif
