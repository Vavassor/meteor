#ifndef INTERPOLATION_H
#define INTERPOLATION_H

float smooth_step(float edge1, float edge2, float t);
float smoother_step(float edge1, float edge2, float t);
float cubic_interpolate(float y0, float y1, float y2, float y3, float t);
float quad_interpolate(float x, float y_1, float y0, float y1);
float catmull_rom(float y0, float y1, float y2, float y3, float t);
float hermite_interpolate(float y0, float y1, float y2, float y3, float t, float tension, float bias);

inline float lerp(float v0, float v1, float t)
{
	return v0 + t * (v1 - v0);
}

// x moves towards t, large N means slow approach, small N is faster
//	 0.0f <= x,t <= 1.0f;     1.0f <= N <= FLT_MAX;
inline float weighted_avg(float x, float t, float N)
{
	return ((x * (N - 1)) + t) / N;
}

#endif
