#ifndef COLLISION_H
#define COLLISION_H

#include "GLMath.h"

struct AABB
{
	vec3 center;
	vec3 extents;
};

struct OBB
{
	vec3 center;
	vec3 extents;
	vec3 axes[3];
};

struct Sphere
{
	vec3 position;
	float radius;
};

class Frustum
{
public:
	vec3 planeNormals[6];
	float planeDots[6];

	Frustum();
	Frustum(const vec3& position, const vec3& viewX, const vec3& viewY, const vec3& viewZ,
		float fov, float ratio, float nearClip, float farClip);
};

struct ConvexRegion
{
	vec3* vertices;
	int numVertices;
};

bool intersect_point_rect(vec2 point, int x, int y, int width, int height);

bool intersect_point_obb(vec3 pointPosition, vec3 obbPosition, quaternion obbOrientation, vec3 obbDimensions);

vec3 intersect_ray_plane(vec3 rayEndpoint, vec3 rayDirection, vec3 planePoint, vec3 planeNormal);
bool intersect_ray_sphere(vec3 rayOrigin, vec3 rayDirection, vec3 sphereCenter, float sphereRadius);
bool intersect_ray_obb(vec3 rayOrigin, vec3 rayDireciton, vec3 obbPosition, quaternion obbOrientation, vec3 obbDimensions);

bool intersect_sphere_sphere(const Sphere& sphere1, const Sphere& sphere2);
bool intersect_aabb_aabb(const AABB& aabb1, const AABB& aabb2);
bool intersect_sphere_aabb(const Sphere& sphere, const AABB& aabb);
bool intersect_obb_obb(const OBB& obbA, const OBB& obbB);
bool intersect_obb_sphere(const OBB& obb, const Sphere& sphere);

void cull_frustum_aabb_list(const Frustum& frustum, AABB* aabbList, int numAABBs, bool* stateList);
void cull_frustum_obb_list(const Frustum& frustum, OBB* obbList, int numOBBs, bool* stateList);

bool intersect_ray_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, vec3* intersect);

#endif
