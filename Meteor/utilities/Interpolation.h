#ifndef INTERPOLATION_H
#define INTERPOLATION_H

extern inline float lerp(float v0, float v1, float t);
float smooth_step(float edge1, float edge2, float t);
float smoother_step(float edge1, float edge2, float t);
extern inline float weighted_avg(float x, float t, float weight);
float cubic_interpolate(float y0, float y1, float y2, float y3, float t);
float quad_interpolate(float x, float y_1, float y0, float y1);
float catmull_rom(float y0, float y1, float y2, float y3, float t);
float hermite_interpolate(float y0, float y1, float y2, float y3, float t, float tension, float bias);

#endif
