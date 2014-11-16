#include "RandomDistribution.h"

#include <math.h>

#define M_PI    3.14159265358979323846

namespace random {

// DISCRETE DISTRIBUTIONS
//-------------------------------------------------------------------------------------------------

/*
 *      Generator         Range (x)     Mean         Variance
 *
 *      Bernoulli         x = 0,1       p            p*(1-p)
 *      Binomial          x = 0,...,n   n*p          n*p*(1-p)
 *      Equilikely        x = a,...,b   (a+b)/2      ((b-a+1)*(b-a+1)-1)/12
 *      Geometric         x = 0,...     p/(1-p)      p/((1-p)*(1-p))
 *      Pascal            x = 0,...     n*p/(1-p)    n*p/((1-p)*(1-p))
 *      Poisson           x = 0,...     m            m
 */

// use 0.0 < p < 1.0
long bernoulli(double p)
{
	return (uniform_real() < 1.0 - p)? 0 : 1;
}

// use n > 0 and 0.0 < p < 1.0
long binomial(long k, double p)
{
	long x = 0;
	for(long i = 0; i < k; ++i)
		x += bernoulli(p);
	return x;
}

// use 0.0 < p < 1.0
long geometric(double p)
{
	return log(uniform_real()) / log(p);
}

// use n > 0 and 0.0 < p < 1.0
long pascal(long k, double p)
{
	long x = 0;
	for(long i = 0; i < k; ++i)
		x += geometric(p);
	return x;
}

// use m > 0
long poisson(double m)
{
	double t = 0.0;
	long x = 0;
	while(t < m)
	{
		t += -log(uniform_real()); // add exponential(1.0)
		x++;
	}
	return x - 1;
}

// CONTINUOUS DISTRIBUTIONS
//-------------------------------------------------------------------------------------------------

/*
 *      Generator         Range (x)     Mean            Variance
 *
 *      Exponential       x > 0         m               m*m
 *      Erlang            x > 0         n*b             n*b*b
 *      Gaussian          all x         m               s*s
 *      Lognormal         x > 0         exp(a+0.5*b*b)  (exp(b*b)-1)*exp(2*a+b*b)
 *      Chi Squared       x > 0         n               2*n
 */

// use lambda > 0
double exponential(double lambda)
{
	return -log(uniform_real()) / lambda;
}

// use k > 0, lambda > 0
double erlang(int k, double lambda)
{
	double x = 0.0;
	for(int i = 0; i < k; ++i)
		x += exponential(lambda);
	return x;
}

/*
 * Uses a very accurate approximation of the normal idf due to Odeh & Evans,
 * J. Applied Statistics, 1974, vol 23, pp 96-97.
 *
 * alternate version uses two random numbers:
 *   return sqrt(-2.0 * log(random_uniform())) * sin(M_TAU * uniform_real());
 */
static double random_normal()
{
	const double p0 = 0.322232431088;     const double q0 = 0.099348462606;
	const double p1 = 1.0;                const double q1 = 0.588581570495;
	const double p2 = 0.342242088547;     const double q2 = 0.531103462366;
	const double p3 = 0.204231210245e-1;  const double q3 = 0.103537752850;
	const double p4 = 0.453642210148e-4;  const double q4 = 0.385607006340e-2;

	double u = uniform_real();

	double t = (u < 0.5)? sqrt(-2.0 * log(u)) : sqrt(-2.0 * log(1.0 - u));

	double p = p0 + t * (p1 + t * (p2 + t * (p3 + t * p4)));
	double q = q0 + t * (q1 + t * (q2 + t * (q3 + t * q4)));

	return (u < 0.5)? (p / q - t) : (t - p / q);
}

// use sigma > 0.0
double gaussian(double mu, double sigma)
{
	return mu + sigma * random_normal();
}

// use k > 0
double chi_squared(int k)
{
    double w = 0.0;
    for(int i = 0; i < k; ++i)
    {
    	double z = random_normal();
    	w += z * z;
    }
    return w;
}

// use gamma > 0.0
double cauchy(double mu, double gamma)
{
	return mu + gamma * tan(M_PI * (uniform_real() - 0.5));
}

double gamma(double theta, double kappa)
{
	int int_kappa = (int)kappa;
	double frac_kappa = kappa - (double)int_kappa;

	// integer part
	double x_int = 0.0;
	for(int i = 0; i < int_kappa; ++i)
	   x_int += -log(uniform_real()); // add exponential(1.0)

	// fractional part
	double x_frac;
	if(fabs(frac_kappa) < 0.01)
		x_frac = 0.0;
	else
	{
		double b = (exp(1.0) + frac_kappa) / exp(1.0);
		while(true)
		{
			double u = uniform_real();
			double p = b * u;

			double uu = uniform_real();

			if(p <= 1.0)
			{
				x_frac = pow(p, 1.0 / frac_kappa);
				if(uu <= exp(-x_frac)) break;
			}
			else
			{
				x_frac = -log((b - p) / frac_kappa);
				if(uu <= pow(x_frac, frac_kappa - 1.0)) break;
			}
		}
	}

	return (x_int + x_frac) * theta;
}

// use sigma > 0.0
double log_normal(double mu, double sigma)
{
	return exp(mu + sigma * random_normal());
}

// use lambda > 0.0
double inverse_gaussian(double mu, double lambda)
{
	double s = random_normal();
	double t = s * s; // chi_squared(1)
	double u = mu + 0.5 * t * mu * mu / lambda - (0.5 * mu / lambda)
	         * sqrt(4.0 * mu * lambda * t + mu * mu * t * t);
	double v = uniform_real();

	return (v < mu / (mu + u))? u : mu * mu / u;
}

} // namespace random
