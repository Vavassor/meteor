#include "Camera.h"

#include "utilities/Maths.h"

#include <math.h>

Camera::Camera():
	mode(PERSPECTIVE),

	focus(VEC3_ZERO),
	position(VEC3_ZERO),

	orientation(quat_from_euler(M_PI / 6.0f, 0.0f, 0.0f)),
	targetOrientation(QUAT_I),
	rotation(QUAT_I),

	fov(90.0f),
	nearPlane(0.1f),
	farPlane(500.0f),

	rotationTime(0),
	perspOffset(vec3(0.0f, 75.0f, -100.0f)),
	velocity(VEC3_ZERO),
	acceleration(VEC3_ZERO)
{}

void Camera::SwitchToPerspective(float nearClip, float farClip, float fieldOfView)
{
	fov = fieldOfView;
	nearPlane = nearClip;
	farPlane = farClip;

	mode = PERSPECTIVE;
	position = focus + perspOffset;
	orientation = quat_from_euler(M_PI / 6.0f, 0.0f, 0.0f);
	
	ResetZoom();
}

void Camera::SwitchToOrtho()
{
	mode = ORTHOGRAPHIC;
	orientation = quat_from_euler(atan(1.0f / 2.0f), -M_PI / 4.0f, 0.0f);

	ResetPosition(focus);
	ResetZoom();
}

void Camera::Move(vec3 movement)
{
	focus += movement;
	position += movement;
}

void Camera::Rotate(vec2 xyDelta)
{
	quaternion turnX = quat_from_euler(0.0f, xyDelta.x, 0.0f);
	quaternion turnY = quat_from_euler(xyDelta.y, 0.0f, 0.0f);

	rotation = turnX * rotation * turnY;
	position = focus + rotation * perspOffset;

	orientation = turnX * orientation * turnY;
}

void Camera::SetFocus(vec3 focusPosition)
{
	focus = focusPosition;
}

void Camera::Zoom(float zDelta)
{
	float zoom = -zDelta * 0.012f + 1.0f;
	perspOffset *= zoom;
}

void Camera::ResetZoom()
{
	perspOffset = vec3(0.0f, 75.0f, -100.0f);
}

void Camera::ResetPosition(vec3 pos)
{
	focus = pos;
	position = pos;
}

CameraData Camera::Update(double deltaTime)
{
	//rotation, physics
	const float t = 1.1f;
	const int MIN_VELOCITY = 1;
	const float Camera_MaxDelta[] = { 16, 16, 16, -16, -16, -16 };
	
	vec3 focusMove = VEC3_ZERO;
	switch(mode)
	{
		case ORTHOGRAPHIC:
		{
			focusMove = rotation * perspOffset;
			focusMove += position;
			break;
		}
		case PERSPECTIVE:
		{
			focusMove = position - rotation * perspOffset;
			break;
		}
	}

	vec3 delta = focus - focusMove;
	acceleration = (2 * (delta - velocity * t)) / (t * t);

	velocity += acceleration * deltaTime * 0.00625f;
	position += velocity * deltaTime * 0.00625f;
		
	vec3 direction = orientation * UNIT_Z;
	if(mode == ORTHOGRAPHIC) direction = -direction;

	// output resulting data for renderer use
	CameraData out = {};
	out.position = position;
	out.nearPlane = nearPlane;
	out.farPlane = farPlane;
	out.fov = fov;
	out.isOrtho = mode == ORTHOGRAPHIC;

	{
		vec3 reference = position + direction;
		vec3 viewZ = normalize(position - reference);
		vec3 viewX = normalize(cross(UNIT_Y, viewZ));
		vec3 viewY = cross(viewZ, viewX);

		out.viewX = viewX;
		out.viewY = viewY;
		out.viewZ = viewZ;
	}

	return out;
}
