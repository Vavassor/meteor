#include "Color.h"

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

vec3 rgb_to_hsv(vec3 rgb)
{
	vec3 hsv = VEC3_ZERO;

	// find smallest/largest componenets of rgb
	float min = rgb.g <= rgb.b ? 
		(rgb.r <= rgb.g ? rgb.r : rgb.g) : 
		(rgb.r <= rgb.b ? rgb.r : rgb.b);
	float max = rgb.g >= rgb.b ? 
		(rgb.r >= rgb.g ? rgb.r : rgb.g) : 
		(rgb.r >= rgb.b ? rgb.r : rgb.b);
	hsv.z = max;
	
	// calculate hue and sat from spread and angle of colors
	float delta = max - min;
	if(delta != 0.0f)
	{
		if (max == rgb.r)
        {
            hsv.x = fmod(rgb.g - rgb.b / delta, 6.0f);
        }
        else if (max == rgb.g)
        {
            hsv.x = (rgb.b - rgb.r) / delta + 2.0f;
        }
        else //if(max == rgb.b)
        {
            hsv.x = (rgb.r - rgb.g) / delta + 4.0f;
        }
        hsv.x *= 60.0f;
        hsv.y = delta / (1.0f - fabs(2.0f * hsv.x - 1.0f));
	}
	return hsv;
}

vec3 hsv_to_rgb(vec3 hsv)
{
	static const float HUE_UPPER_LIMIT = 360.0f;
	vec3 rgb = VEC3_ZERO;

	float c = hsv.z * hsv.y;
    float x = c * (1.0f - fabs(fmod(hsv.x / 60.0f, 2.0f) - 1.0f));
    float m = hsv.z - c;

    if (hsv.x >= 0.0f && hsv.x < (HUE_UPPER_LIMIT / 6.0f))
    {
        rgb = vec3(c + m, x + m, m);
    }
    else if (hsv.x >= (HUE_UPPER_LIMIT / 6.0f) && hsv.x < (HUE_UPPER_LIMIT / 3.0f))
    {
        rgb = vec3(x + m, c + m, m);
    }
    else if (hsv.x >= (HUE_UPPER_LIMIT / 3.0f) && hsv.x < (HUE_UPPER_LIMIT / 2.0f))
    {
        rgb = vec3(m, c + m, x + m);
    }
    else if (hsv.x >= (HUE_UPPER_LIMIT / 2.0f) && hsv.x < (2.0f * HUE_UPPER_LIMIT / 3.0f))
    {
        rgb = vec3(m, x + m, c + m);
    }
    else if (hsv.x >= (2.0 * HUE_UPPER_LIMIT / 3.0f) && hsv.x < (5.0 * HUE_UPPER_LIMIT / 6.0f))
    {
        rgb = vec3(x + m, m, c + m);
    }
    else if (hsv.x >= (5.0f * HUE_UPPER_LIMIT / 6.0f) && hsv.x <= HUE_UPPER_LIMIT)
    {
        rgb = vec3(c + m, m, x + m);
    }
    else
    {
        rgb = vec3(m, m, m);
    }
    return rgb;
}