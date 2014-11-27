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

	bool GetIsOrtho() const { return mode == ORTHOGRAPHIC; }

	void SwitchToPerspective(float nearClip, float farClip, float fieldOfView = 90.0f);
	void SwitchToOrtho();
	void Move(vec3 movement);
	void Rotate(vec2 xyDelta);
	void SetFocus(vec3 focusPosition);
	void Zoom(float zDelta);
	void ResetZoom();
	void ResetPosition(vec3 pos);
	CameraData Update(double deltaTime);

private:
	quaternion targetOrientation;
	vec3 velocity, acceleration;
	vec3 perspOffset;
	float rotationTime;
};

#endif
