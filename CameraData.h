#ifndef CAMERA_DATA_H
#define CAMERA_DATA_H

#include "utilities/GLMath.h"

struct CameraData
{
	vec3 position;
	vec3 right, up, forward;
	float near_plane, far_plane;
	float field_of_view;
	bool orthographic;
};

#endif
