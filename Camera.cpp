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

void Camera::Switch_To_Perspective(float nearClip, float farClip, float fieldOfView)
{
	fov = fieldOfView;
	nearPlane = nearClip;
	farPlane = farClip;

	mode = PERSPECTIVE;
	position = focus + perspOffset;
	orientation = quat_from_euler(M_PI / 6.0f, 0.0f, 0.0f);
	
	Reset_Zoom();
}

void Camera::Switch_To_Orthographic()
{
	mode = ORTHOGRAPHIC;
	orientation = quat_from_euler(atan(1.0f / 2.0f), -M_PI / 4.0f, 0.0f);

	Reset_Position(focus);
	Reset_Zoom();
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

void Camera::Set_Focus(vec3 focusPosition)
{
	focus = focusPosition;
}

void Camera::Zoom(float zDelta)
{
	float zoom = -zDelta * 0.012f + 1.0f;
	perspOffset *= zoom;
}

void Camera::Reset_Zoom()
{
	perspOffset = vec3(0.0f, 75.0f, -100.0f);
}

void Camera::Reset_Position(vec3 pos)
{
	focus = pos;
	position = pos;
}

void Camera::Update(double deltaTime)
{
	//rotation, physics
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

    const float t = 1.1f;
	vec3 delta = focus - focusMove;
	acceleration = (2 * (delta - velocity * t)) / (t * t);

	velocity += acceleration * deltaTime * 0.00625f;
	position += velocity * deltaTime * 0.00625f;
}

CameraData Camera::Get_Camera_Data() const
{
    vec3 direction = orientation * UNIT_Z;
    if(mode == ORTHOGRAPHIC) direction = -direction;

    // output for renderer use
    CameraData out = {};
    out.position = position;
    out.near_plane = nearPlane;
    out.far_plane = farPlane;
    out.field_of_view = fov;
    out.orthographic = mode == ORTHOGRAPHIC;

    {
        vec3 reference = position + direction;
        vec3 view_z = normalize(position - reference);
        vec3 view_x = normalize(cross(UNIT_Y, view_z));
        vec3 view_y = cross(view_z, view_x);

        out.right = view_x;
        out.up = view_y;
        out.forward = view_z;
    }

    return out;
}