#ifndef CAMERA_DATA_H
#define CAMERA_DATA_H

#include "utilities/GLMath.h"

struct CameraData
{
	mat4x4 projection;
	vec3 position;
	vec3 viewX, viewY, viewZ;
	float nearPlane, farPlane;
	float fov;
	bool isOrtho;
};

#endif
