#ifndef CAMERA_H
#define CAMERA_H

#include "utilities/GLMath.h"

class Camera
{
private:
	enum Mode { ORTHOGRAPHIC, PERSPECTIVE };
	Mode mode;

public:
	mat4x4 view, projection, scale;
	vec3 viewX, viewY, viewZ, focus, position;
	quaternion orientation, targetOrientation, rotation;
	int screenWidth, screenHeight;
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
	void Resize(int width, int height);
	void Tick(double deltaTime);

private:
	float rotationTime;
	vec3 velocity, acceleration;
	vec3 perspOffset;

	void LookAt(vec3 reference, vec3 position);
};

#endif
