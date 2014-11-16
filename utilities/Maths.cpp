#include "Maths.h"

#include <math.h>
#include <float.h>

int mod(int a, int b)
{
	if(b == 0) return a;
	const int c = a % b;
	return (c < 0) ? c + b : c;
}

long long mod_power(int a, int n, int modulus)
{
	long long result = 1;
	while(n > 0)
	{
		if(n & 1) result = (result * a) % modulus;
		a = (a * a) % modulus;
		n >>= 1;
	}
	return result;
}

double binary_log(double x)
{
    return log(x) * M_LOG2E;
}

bool float_equals(float a, float b)
{
    float difference = fabs(a - b);

    a = fabs(a);
    b = fabs(b);
    float largest = (b > a) ? b : a;

    return difference <= largest * FLT_EPSILON
	    || difference < FLT_MIN;
}

static bool prime_witness(int a, int n)
{
	int u = n / 2;
	int t = 1;
	while(!(u & 1))
	{
		u /= 2;
		++t;
	}

	long long curr;
	long long prev = mod_power(a, u, n);
	for(int i = 1; i <= t; ++i)
	{
		curr = (prev * prev) % n;
		if((curr == 1) && (prev != 1) && (prev != n - 1))
			return true;
		prev = curr;
	}
	if(curr != 1) return true;
	return false;
}

bool is_prime(unsigned int m)
{
	if(m < 2) return false;
	if(m == 2) return true;
	if(!(m & 1)) return false;
	if(m % 3 == 0) return m == 3;

	if(m < 1373653)
	{
		for(unsigned int k = 1; 36 * k * k - 12 * k < m; ++k)
		{
			if(m % (6 * k + 1) == 0) return false;
			if(m % (6 * k - 1) == 0) return false;
		}
	}

	if(m < 9080191)
	{
		if(prime_witness(31, m)) return false;
		if(prime_witness(73, m)) return false;
		return true;
	}

	if(prime_witness(2, m)) return false;
	if(prime_witness(7, m)) return false;
	if(prime_witness(61, m)) return false;
	return true;
}

bool is_nan(double n)
{
#if defined(isnan)
	return isnan(n);
#elif defined(_isnan)
	return _isnan(n);
#else
	return n != n;
#endif
}

bool is_infinite(double n)
{
#if defined(isinf)
	return isinf(n);
#elif defined(_finite)
	return !_finite(n);
#else
	return n == n && (n - n) != 0.0;
#endif
}
