#ifndef NOISE_H
#define NOISE_H

double perlin_noise(double x, double y, double z);

double simplex_noise_2d(double x, double y);
double simplex_noise_3d(double x, double y, double z);
double simplex_noise_4d(double x, double y, double z, double w);

double octave_simplex_noise_4d(double numOctaves, double persistence, double scale, double x, double y, double z, double w);

#endif
