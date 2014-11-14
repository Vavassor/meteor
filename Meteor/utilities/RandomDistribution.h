#ifndef RANDOM_DISTRIBUTION_H
#define RANDOM_DISTRIBUTION_H

#include "RandomUniform.h"

namespace random {

//--- DISCRETE DISTRIBUTIONS ----------------------------------------------------------------------

long bernoulli(double p);
long binomial(long k, double p);
long geometric(double p);
long pascal(long k, double p);
long poisson(double m);

//--- CONTINUOUS DISTRIBUTIONS --------------------------------------------------------------------

double exponential(double lambda);
double erlang(int k, double lambda);
double gaussian(double mu, double sigma);
double chi_squared(int k);
double cauchy(double mu, double gamma);
double gamma(double theta, double kappa);
double log_normal(double mu, double sigma);
double inverse_gaussian(double mu, double lambda);

} // namespace random

#endif
