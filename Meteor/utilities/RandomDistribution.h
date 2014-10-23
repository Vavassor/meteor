#ifndef RANDOM_DISTRIBUTION_H
#define RANDOM_DISTRIBUTION_H

#include "RandomUniform.h"

//--- DISCRETE DISTRIBUTIONS ----------------------------------------------------------------------

long random_bernoulli(double p);
long random_binomial(long k, double p);
long random_geometric(double p);
long random_pascal(long k, double p);
long random_poisson(double m);

//--- CONTINUOUS DISTRIBUTIONS --------------------------------------------------------------------

double random_exponential(double lambda);
double random_erlang(int k, double lambda);
double random_gaussian(double mu, double sigma);
double random_chi_squared(int k);
double random_cauchy(double mu, double gamma);
double random_gamma(double theta, double kappa);
double random_log_normal(double mu, double sigma);
double random_inverse_gaussian(double mu, double lambda);

#endif
