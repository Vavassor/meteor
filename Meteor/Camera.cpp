#include "Camera.h"

#include "utilities/Maths.h"

#include <math.h>

Camera::Camera():
	mode(PERSPECTIVE),

	view(MAT_I),
	projection(MAT_I),
	scale(scale_matrix(1.0f, 1.0f, 1.0f)),

	viewX(UNIT_X),
	viewY(UNIT_Y),
	viewZ(UNIT_Z),

	focus(VEC3_ZERO),
	position(VEC3_ZERO),

	orientation(quat_from_euler(M_PI / 6.0f, 0.0f, 0.0f)),
	targetOrientation(QUAT_I),
	rotation(QUAT_I),

	screenWidth(0),
	screenHeight(0),

	fov(90.0f),
	nearPlane(0.1f),
	farPlane(500.0f),

	rotationTime(0),
	perspOffset(vec3(0.0f, 75.0f, -100.0f)),
	velocity(VEC3_ZERO),
	acceleration(VEC3_ZERO)
{}

void Camera::LookAt(vec3 reference, vec3 position)
{
	viewZ = normalize(position - reference);
	viewX = normalize(cross(UNIT_Y, viewZ));
	viewY = cross(viewZ, viewX);
}

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
	switch(mode)
	{
		case ORTHOGRAPHIC:
		{
			float zoom = zDelta * 0.012f + 1.0f;
			scale = scale_matrix(zoom, zoom, 1.0f);
			projection = scale * projection;
			break;
		}
		case PERSPECTIVE:
		{
			float zoom = -zDelta * 0.012f + 1.0f;
			perspOffset *= zoom;
			break;
		}
	}
}

void Camera::ResetZoom()
{
	switch(mode)
	{
		case ORTHOGRAPHIC:
			projection = orthogonal_projection_matrix(0, screenWidth, 0, screenHeight, -4.0f * screenHeight, 4.0f * screenHeight); break;
		case PERSPECTIVE:
			projection = perspective_projection_matrix(fov, screenWidth, screenHeight, nearPlane, farPlane); break;
	}
}

void Camera::ResetPosition(vec3 pos)
{
	focus = pos;
	if(mode == ORTHOGRAPHIC)
	{
		float halfHypotenuse = 0.5f * sqrt((screenWidth * screenWidth / 4.0f) + (screenWidth * screenWidth / 4.0f));
		position = focus - vec3(halfHypotenuse, screenHeight / 2.0f, halfHypotenuse);
	}
}

void Camera::Resize(int width, int height)
{
	screenWidth = width;
	screenHeight = height;
	if(mode == ORTHOGRAPHIC)
	{
		position = focus - vec3(screenWidth / 2.0f, screenHeight, 0.0f);
	}
}

void Camera::Tick(double deltaTime)
{
	view = view_matrix(viewX, viewY, viewZ, position);

	//rotation, physics
	const float t = 1.1f;
	const int MIN_VELOCITY = 1;
	const float Camera_MaxDelta[] = { 16, 16, 16, -16, -16, -16 };
	
	vec3 focusMove = VEC3_ZERO;
	switch(mode)
	{
		case ORTHOGRAPHIC:
		{
			float halfHypotenuse = 0.5f * sqrt((screenWidth * screenWidth / 4.0f) + (screenWidth * screenWidth / 4.0f));
			focusMove = rotation * vec3(halfHypotenuse, screenHeight / 2.0f, halfHypotenuse);
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
	LookAt(position + direction, position);
}
