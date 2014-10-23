#include "Color.h"

#include <math.h>

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

#define HUE_UPPER_LIMIT 360.0f

vec3 hsv_to_rgb(vec3 hsv)
{
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
