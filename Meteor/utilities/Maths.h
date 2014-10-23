#ifndef MATHS_H
#define MATHS_H

int mod(int a, int b);
long long mod_power(int a, int n, int modulus);
bool float_equals(float a, float b);

inline int round_int(double r)
{
	return (r > 0.0) ? r + 0.5 : r - 0.5;
}

inline float clamp(float x, float min, float max)
{
    return (x < min) ? min : ((x > max) ? max : x);
}

#if defined(_MSC_VER)
inline double log2(double x)
{
    return log(x) * M_LOG2E;
}
#endif

bool is_prime(unsigned int m);
bool is_nan(double n);
bool is_infinite(double n);

inline bool is_power_of_two(int x)
{
	return x > 0 && (x & (x - 1)) == 0;
}

//--- INFINITY CONSTANTS ----------------------------------------------------------------

#if defined(_MSC_VER)
#define POS_INF_F 0x1.0p255f		// 0x7f800000
#define NEG_INF_F -POS_INF_F		// 0xff800000
#define POS_INF_D 0x1.0p2047		// 0x7ff0000000000000
#define NEG_INF_D -POS_INF_D		// 0xfff0000000000000

#else
static union { unsigned long x; float y; } pos_huge_valf = { 0x7f800000 };
static union { unsigned long x; float y; } neg_huge_valf = { 0xff800000 };
static union { unsigned long long x; double y; } pos_huge_val = { 0x7f80000000000000 };
static union { unsigned long long x; double y; } neg_huge_val = { 0xff80000000000000 };

#define POS_INF_F (pos_huge_valf.y)
#define NEG_INF_F (neg_huge_valf.y)
#define POS_INF_D (pos_huge_val.y)
#define NEG_INF_D (neg_huge_val.y)
#endif

//--- MATHEMATICAL CONSTANTS ------------------------------------------------------------

#define M_E        2.71828182845904523536
#define M_LOG2E    1.44269504088896340736
#define M_LOG10E   0.43429448190325182765
#define M_LN2      0.69314718055994530942
#define M_LN10     2.30258509299404568402
#define M_PI       3.14159265358979323846
#define M_PI_2     1.57079632679489661923
#define M_PI_4     0.78539816339744830962
#define M_1_PI     0.31830988618379067154
#define M_2_PI     0.63661977236758134308
#define M_2_SQRTPI 1.12837916709551257390
#define M_SQRT2    1.41421356237309504880
#define M_SQRT1_2  0.70710678118654752440

#define M_TAU      6.28318530717958647692
#define M_PHI      1.61803398874989484820

#endif
