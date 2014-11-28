#ifndef CAMERA_H
#define CAMERA_H

#include "CameraData.h"

#include "utilities/GLMath.h"

class Camera
{
private:
	enum Mode { ORTHOGRAPHIC, PERSPECTIVE };
	Mode mode;

public:
	vec3 focus, position;
	quaternion orientation, rotation;
	float fov, nearPlane, farPlane;

	Camera();

	bool Is_Orthographic() const { return mode == ORTHOGRAPHIC; }
    CameraData Get_Camera_Data() const;

	void Switch_To_Perspective(float nearClip, float farClip, float fieldOfView = 90.0f);
	void Switch_To_Orthographic();
	void Move(vec3 movement);
	void Rotate(vec2 xyDelta);
	void Set_Focus(vec3 focusPosition);
	void Zoom(float zDelta);
	void Reset_Zoom();
	void Reset_Position(vec3 pos);
	void Update(double deltaTime);

private:
	quaternion targetOrientation;
	vec3 velocity, acceleration;
	vec3 perspOffset;
	float rotationTime;
};

#endif
