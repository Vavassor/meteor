#include "Interpolation.h"

#include "Maths.h"

inline float lerp(float v0, float v1, float t)
{
	return v0 + t * (v1 - v0);
}

// edge1 <= t <= edge2
float smooth_step(float edge1, float edge2, float t)
{
	float y = (t - edge1) / (edge2 - edge1);
	y = clamp(y, 0.0f, 1.0f);
	return y * y * (3 - 2 * y);
}

// edge1 <= t <= edge2
float smoother_step(float edge1, float edge2, float t)
{
	float y = (t - edge1) / (edge2 - edge1);
	y = clamp(y, 0.0f, 1.0f);
	return y * y * y * (y * (y * 6 - 15) + 10);
}

// x moves towards t, large N means slow approach, small N is faster
//	 0.0f <= x,t <= 1.0f;     1.0f <= N <= FLT_MAX;
inline float weighted_avg(float x, float t, float N)
{
	return ((x * (N - 1)) + t) / N;
}

// y0-3 control points affecting t
float cubic_interpolate(float y0, float y1, float y2, float y3, float t)
{
	float a0 = y3 - y2 - y0 + y1;
	float a1 = y0 - y1 - a0;
	float a2 = y2 - y0;
	return a0 * t * t * t + a1 * t * t + a2 * t + y1;
}

// parabola through 3 points, -1 < x < 1
float quad_interpolate(float x, float y_1, float y0, float y1)
{
    if(x <= -1) return y_1;
    if(x >= 1) return y1;
    float l = y0 - x * (y_1 - y0);
    float r = y0 + x * (y1 - y0);
    return (l + r + x * (r - l)) / 2;
}

float catmull_rom(float y0, float y1, float y2, float y3, float t)
{
	return y1 + 0.5f * t * (y2 - y0 + t * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3 + t * (3.0f * (y1 - y2) + y3 - y0)));
}

/*
 *	y0-3 control points affecting x, with tension changing curvature, bias changing offset
 *	Tension: 1 is high, 0 normal, -1 is low
 *	Bias: 0 is even, positive is towards first segment, negative towards the other
 */
float hermite_interpolate(float y0, float y1, float y2, float y3, float t, float tension, float bias)
{
	float mu2 = t * t;
	float mu3 = mu2 * t;

	float m0  = (y1 - y0) * (1 + bias) * (1 - tension) / 2;
		  m0 += (y2 - y1) * (1 - bias) * (1 - tension) / 2;
	float m1  = (y2 - y1) * (1 + bias) * (1 - tension) / 2;
		  m1 += (y3 - y2) * (1 - bias) * (1 - tension) / 2;

	float a0 = 2 * mu3 - 3 * mu2 + 1;
	float a1 = mu3 - 2 * mu2 + t;
	float a2 = mu3 - mu2;
	float a3 = -2 * mu3 + 3 * mu2;

	return a0 * y1 + a1 * m0 + a2 * m1 + a3 * y2;
}

#if 0
vec3* catmull_rom_spline(vec3* points, int numPoints, int samples)
{
	if(numPoints < 2) return nullptr;

	vec3* results = new vec3[numPoints * samples];
	for(int n = 0; n < numPoints - 1; n++)
	{
	    for(int i = 0; i < samples; i++)
	    {
			vec3 p0 = points[(n == 0) ? n : n - 1];
			vec3 p1 = points[n];
			vec3 p2 = points[n + 1];
			vec3 p3 = points[(n == numPoints - 2) ? n + 1 : n + 2];
			float t = (1.0f / samples) * i;

			float t0 = ((-t + 2.0f) * t - 1.0f) * t * 0.5f;
			float t1 = (((3.0f * t - 5.0f) * t) * t + 2.0f) * 0.5f;
			float t2 = ((-3.0f * t + 4.0f) * t + 1.0f) * t * 0.5f;
			float t3 = (t - 1.0f) * t * t * 0.5f;

			results[n * samples + i] = p0 * t0 + p1 * t1 + p2 * t2 + p3 * t3;
	    }
	}
	return results;
}
#endif
