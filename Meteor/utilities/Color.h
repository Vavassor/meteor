#ifndef __COLOR_H__
#define __COLOR_H__

#include "GLMath.h"
#include "DataTypes.h"

vec3 rgb_to_hsv(vec3 rgb);
vec3 hsv_to_rgb(vec3 hsv);

inline uint8_t get_alpha(uint32_t color)
{
	return color >> 24 & 0xFF;
}

inline uint8_t get_red(uint32_t color)
{
	return color >> 16 & 0xFF;
}

inline uint8_t get_green(uint32_t color)
{
	return color >> 8 & 0xFF;
}

inline uint8_t get_blue(uint32_t color)
{
	return color & 0xFF;
}

inline uint32_t combine_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return uint32_t(r) << 16 | uint32_t(g) << 8 | b;
}

inline vec3 rgb_to_vec(uint32_t color)
{
	float red = float(get_red(color)) / 255.0f;
	float green = float(get_green(color)) / 255.0f;
	float blue = float(get_blue(color)) / 255.0f;
	return vec3(red, green, blue);
}

inline vec4 rgba_to_vec(uint32_t color)
{
	float red = float(get_red(color)) / 255.0f;
	float green = float(get_green(color)) / 255.0f;
	float blue = float(get_blue(color)) / 255.0f;
	float alpha = float(get_alpha(color)) / 255.0f;
	return vec4(red, green, blue, alpha);
}

inline unsigned int vec_to_rgb(vec3 rgb)
{
	uint8_t red = rgb.r * 255.0f;
	uint8_t green = rgb.g * 255.0f;
	uint8_t blue = rgb.b * 255.0f;
	return combine_rgb(red, green, blue);
}

#endif
