#ifndef __COLOR_H__
#define __COLOR_H__

#include "GLMath.h"
#include "DataTypes.h"

extern inline uint8_t get_alpha(uint32_t color);
extern inline uint8_t get_red(uint32_t color);
extern inline uint8_t get_green(uint32_t color);
extern inline uint8_t get_blue(uint32_t color);
extern inline uint32_t combine_rgb(uint8_t r, uint8_t g, uint8_t b);
extern inline vec3 rgb_to_vec(uint32_t color);
extern inline vec4 rgba_to_vec(uint32_t color);
extern inline uint32_t vec_to_rgb(vec3 rgb);

vec3 rgb_to_hsv(vec3 rgb);
vec3 hsv_to_rgb(vec3 hsv);

#endif