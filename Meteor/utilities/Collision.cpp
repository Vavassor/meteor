#include "Collision.h"

#include "Maths.h"

enum FrustumPlanes
{
	FRUSTUM_TOP, FRUSTUM_BOTTOM,
	FRUSTUM_LEFT, FRUSTUM_RIGHT,
	FRUSTUM_NEAR, FRUSTUM_FAR,
};

Frustum::Frustum()
{
	for(int i = 0; i < 6; i++)
		planeNormals[i] = VEC3_ZERO;
	for(int i = 0; i < 6; i++)
		planeDots[i] = 0.0f;
}

Frustum::Frustum(const vec3& position, const vec3& viewX, const vec3& viewY, const vec3& viewZ,
	float fov, float ratio, float nearClip, float farClip)
{
	vec3 nc = position - viewZ * nearClip;
	vec3 fc = position - viewZ * farClip;

	planeNormals[FRUSTUM_NEAR] = -viewZ;
	planeNormals[FRUSTUM_FAR] = viewZ;

	float nearHeight = nearClip * tan((M_PI / 180.0f) * fov * 0.5f);
	float nearWidth = nearHeight * ratio;

	vec3 aux, normal;

	aux = (nc + viewY * nearHeight) - position;
	aux = normalize(aux);
	normal = cross(aux, viewX);
	planeNormals[FRUSTUM_TOP] = normalize(normal);
	
	aux = (nc - viewY * nearHeight) - position;
	aux = normalize(aux);
	normal = cross(viewX, aux);
	planeNormals[FRUSTUM_BOTTOM] = normalize(normal);
	
	aux = (nc - viewX * nearWidth) - position;
	aux = normalize(aux);
	normal = cross(aux, viewY);
	planeNormals[FRUSTUM_LEFT] = normalize(normal);

	aux = (nc + viewX * nearWidth) - position;
	aux = normalize(aux);
	normal = cross(viewY, aux);
	planeNormals[FRUSTUM_RIGHT] = normalize(normal);
	
	vec3 planePositions[6];
	planePositions[FRUSTUM_TOP] = nc + viewY * nearHeight;
	planePositions[FRUSTUM_BOTTOM] = nc - viewY * nearHeight;
	planePositions[FRUSTUM_LEFT] = nc - viewX * nearWidth;
	planePositions[FRUSTUM_RIGHT] = nc + viewX * nearWidth;
	planePositions[FRUSTUM_NEAR] = nc;
	planePositions[FRUSTUM_FAR] = fc;

	for(int i = 0; i < 6; i++) 
		planeDots[i] = dot(planeNormals[i], planePositions[i]);
}

static vec3 get_right_axis(const quaternion& q)
{
	return vec3(1 - 2 * (q.y * q.y + q.z * q.z),
					2 * (q.x * q.y + q.w * q.z),
					2 * (q.x * q.z - q.w * q.y));
}

static vec3 get_up_axis(const quaternion& q)
{
	return vec3(2 * (q.x * q.y - q.w * q.z),
			1 - 2 * (q.x * q.x + q.z * q.z),
                2 * (q.y * q.z + q.w * q.x));
}

static vec3 get_forward_axis(const quaternion& q)
{
	return vec3(2 * (q.x * q.z + q.w * q.y), 
				2 * (q.y * q.z - q.w * q.x),
			1 - 2 * (q.x * q.x + q.y * q.y));
}

bool intersect_point_rect(vec2 point, int x, int y, int width, int height)
{
	return point.x > x
		&& point.x < x + width
		&& point.y > y
		&& point.y < y + height;
}

bool intersect_point_obb(vec3 pointPosition, vec3 obbPosition, quaternion obbOrientation, vec3 obbDimensions)
{
	vec3 pointPos = invert_quat(obbOrientation) * (pointPosition - obbPosition);
	return pointPos.x >= 0 && pointPos.x <= obbDimensions.x
		&& pointPos.y >= 0 && pointPos.y <= obbDimensions.y
		&& pointPos.z >= 0 && pointPos.z <= obbDimensions.z;
}

vec3 intersect_ray_plane(vec3 rayOrigin, vec3 rayDirection, vec3 planePoint, vec3 planeNormal)
{
	float t = dot(planeNormal, planePoint - rayOrigin)
			/ dot(planeNormal, rayDirection);
	return rayOrigin + (rayDirection * t);
}

bool intersect_ray_sphere(vec3 rayOrigin, vec3 rayDirection, vec3 sphereCenter, float sphereRadius)
{
	vec3 translation = rayOrigin - sphereCenter;
    float a0 = dot(translation, translation) - sphereRadius * sphereRadius;
   
	// is inside the sphere
	if(a0 <= 0.0f) return true;

    // else: outside the sphere
    float a1 = dot(rayDirection, translation);
    if(a1 >= 0.0f) return false;

    // Quadratic has a real root if discriminant is nonnegative.
    return a1 * a1 >= a0;
}

bool intersect_ray_obb(vec3 rayOrigin, vec3 rayDireciton, vec3 obbPosition, quaternion obbOrientation, vec3 obbDimensions)
{
	vec3 WdU, AWdU, DdU, ADdU, AWxDdU;
	float RHS;

	vec3 diff = rayOrigin - obbPosition;
	const float extents[3] = { obbDimensions.x / 2.0f, obbDimensions.y / 2.0f, obbDimensions.z / 2.0f };

	vec3 rightAxis = get_right_axis(obbOrientation);
    WdU.x = dot(rayDireciton, rightAxis);
    AWdU.x = fabs(WdU.x);
    DdU.x = dot(diff, rightAxis);
    ADdU.x = fabs(DdU.x);
    if(ADdU.x > extents[0] && DdU.x * WdU.x >= 0.0f)
        return false;

	vec3 upAxis = get_up_axis(obbOrientation);
    WdU.y = dot(rayDireciton, upAxis);
    AWdU.y = fabs(WdU.y);
    DdU.y = dot(diff, upAxis);
    ADdU.y = fabs(DdU.y);
    if (ADdU.y > extents[1] && DdU.y * WdU.y >= 0.0f)
        return false;

	vec3 forwardAxis = get_forward_axis(obbOrientation);
    WdU.z = dot(rayDireciton, forwardAxis);
    AWdU.z = fabs(WdU.z);
    DdU.z = dot(diff, forwardAxis);
    ADdU.z = fabs(DdU.z);
    if (ADdU.z > extents[2] && DdU.z * WdU.z >= 0.0f)
        return false;

	vec3 WxD = cross(rayDireciton, diff);

    AWxDdU.x = fabs(dot(WxD, rightAxis));
    RHS = extents[1] * AWdU.z + extents[2] * AWdU.y;
    if (AWxDdU.x > RHS)
        return false;

    AWxDdU.y = fabs(dot(WxD, upAxis));
    RHS = extents[0] * AWdU.z + extents[2] * AWdU.x;
    if (AWxDdU.y > RHS)
        return false;

    AWxDdU.z = fabs(dot(WxD, forwardAxis));
    RHS = extents[0] * AWdU.y + extents[1] * AWdU.x;
    if (AWxDdU.z > RHS)
        return false;

    return true;
}

bool intersect_sphere_sphere(const Sphere& sphere1, const Sphere& sphere2)
{
	float distance = sphere1.radius + sphere2.radius;
	vec3 translation = sphere1.position - sphere2.position;
	return dot(translation, translation) <= distance * distance;
}

bool intersect_aabb_aabb(const AABB& aabb1, const AABB& aabb2)
{
	return fabs(aabb1.center.x - aabb2.center.x) < (aabb1.extents.x + aabb2.extents.x)
		&& fabs(aabb1.center.y - aabb2.center.y) < (aabb1.extents.y + aabb2.extents.y)
		&& fabs(aabb1.center.z - aabb2.center.z) < (aabb1.extents.z + aabb2.extents.z);
}

bool intersect_sphere_aabb(const Sphere& sphere, const AABB& aabb)
{
	vec3 aabbMin = aabb.center - aabb.extents;
	vec3 aabbMax = aabb.center + aabb.extents;

	// find the closest point to the sphere on aabb
	vec3 closestPoint;
	closestPoint.x = (sphere.position.x < aabbMin.x)? aabbMin.x : (sphere.position.x > aabbMax.x)? aabbMax.x : sphere.position.x;
    closestPoint.y = (sphere.position.y < aabbMin.y)? aabbMin.y : (sphere.position.y > aabbMax.y)? aabbMax.y : sphere.position.y;
    closestPoint.z = (sphere.position.z < aabbMin.z)? aabbMin.z : (sphere.position.z > aabbMax.z)? aabbMax.z : sphere.position.z;

	// if the closest point is less than the sphere's radius, they collide
	vec3 diff = closestPoint - sphere.position;
	return dot(diff, diff) < sphere.radius * sphere.radius;
}

bool intersect_obb_obb(const OBB& obbA, const OBB& obbB)
{
	// does separating axis test along 15 possible axes

	vec3 EA = obbA.extents;
	vec3 EB = obbB.extents;

	// C transforms the axes of A and B into the 
	// same local space (same orientation)
	float C[3][3];
	float absC[3][3];

	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			C[i][j] = dot(obbA.axes[i], obbB.axes[j]);
			absC[i][j] = fabs(C[i][j]);
		}
	}

	vec3 D = obbB.center - obbA.center;
	vec3 AD = vec3(	dot(obbA.axes[0], D),
					dot(obbA.axes[1], D),
					dot(obbA.axes[2], D));

	// interval and radii of A and B along a potential separating axis
	float t, rA, rB;

	//--- THREE BASIS AXES FOR A ---//
	// axis C0+t*A0
    t = fabs(AD.x);
    rB = EB.x * absC[0][0] + EB.y * absC[0][1] + EB.z * absC[0][2];
	if(t > EA.x + rB) return false;

    // axis C0+t*A1
    t = fabs(AD.y);
    rB = EB.x * absC[1][0] + EB.y * absC[1][1] + EB.z * absC[1][2];
	if(t > EA.y + rB) return false;

    // axis C0+t*A2
    t = fabs(AD.z);
    rB = EB.x * absC[2][0] + EB.y * absC[2][1] + EB.z * absC[2][2];
	if(t > EA.z + rB) return false;

	//--- THREE BASIS AXES FOR B ---//
	// axis C0+t*B0
    t = fabs(dot(obbB.axes[0], D));
    rA = EA.x * absC[0][0] + EA.y * absC[1][0] + EA.z * absC[2][0];
	if(t > rA + EB.x) return false;

    // axis C0+t*B1
    t = fabs(dot(obbB.axes[1], D));
    rA = EA.x * absC[0][1] + EA.y * absC[1][1] + EA.z * absC[2][1];
	if(t > rA + EB.y) return false;

    // axis C0+t*B2
    t = fabs(dot(obbB.axes[2], D));
    rA = EA.x * absC[0][2] + EA.y * absC[1][2] + EA.z * absC[2][2];
    if(t > rA + EB.z) return false;

	//--- NINE CROSS PRODUCT AXES ---//
	// axis C0+t*A0xB0
	t = fabs(AD.z * C[1][0] - AD.y * C[2][0]);
	rA = EA.y * absC[2][0] + EA.z * absC[1][0];
	rB = EB.y * absC[0][2] + EB.z * absC[0][1];
	if(t > rA + rB) return false;

	// axis C0+t*A0xB1
	t = fabs(AD.z * C[1][1] - AD.y * C[2][1]);
	rA = EA.y * absC[2][1] + EA.z * absC[1][1];
	rB = EB.x * absC[0][2] + EB.z * absC[0][0];
	if(t > rA + rB) return false;

	// axis C0+t*A0xB2
	t = fabs(AD.z * C[1][2] - AD.y * C[2][2]);
	rA = EA.y * absC[2][2] + EA.z * absC[1][2];
	rB = EB.x * absC[0][1] + EB.y * absC[0][0];
	if(t > rA + rB) return false;

	// axis C0+t*A1xB0
	t = fabs(AD.x * C[2][0] - AD.z * C[0][0]);
	rA = EA.x * absC[2][0] + EA.z * absC[0][0];
	rB = EB.y * absC[1][2] + EB.z * absC[1][1];
	if(t > rA + rB) return false;

	// axis C0+t*A1xB1
	t = fabs(AD.x * C[2][1] - AD.z * C[0][1]);
	rA = EA.x * absC[2][1] + EA.z * absC[0][1];
	rB = EB.x * absC[1][2] + EB.z * absC[1][0];
	if(t > rA + rB) return false;

	// axis C0+t*A1xB2
	t = fabs(AD.x * C[2][2] - AD.z * C[0][2]);
	rA = EA.x * absC[2][2] + EA.z * absC[0][2];
	rB = EB.x * absC[1][1] + EB.y * absC[1][0];
	if(t > rA + rB) return false;

	// axis C0+t*A2xB0
	t = fabs(AD.y * C[0][0] - AD.x * C[1][0]);
	rA = EA.x * absC[1][0] + EA.y * absC[0][0];
	rB = EB.y * absC[2][2] + EB.z * absC[2][1];
	if(t > rA + rB) return false;

	// axis C0+t*A2xB1
	t = fabs(AD.y * C[0][1] - AD.x * C[1][1]);
	rA = EA.x * absC[1][1] + EA.y * absC[0][1];
	rB = EB.x * absC[2][2] + EB.z * absC[2][0];
	if(t > rA + rB) return false;

	// axis C0+t*A2xB2
	t = fabs(AD.y * C[0][2] - AD.x * C[1][2]);
	rA = EA.x * absC[1][2] + EA.y * absC[0][2];
	rB = EB.x * absC[2][1] + EB.y * absC[2][0];
	if(t > rA + rB) return false;

	return true;
}

/*
bool intersect_obb_sphere(const OBB& obb, const Sphere& sphere)
{
	const vec3 diff = sphere.position - obb.center;

	vec3 pointOnOBB = VEC3_ZERO;
	for(int i = 0; i < 3; i++)
	{
		float dist = dot(diff, obb.axes[i]);

		if(dist > obb.extents[i]) dist = obb.extents[i];
		if(dist < -obb.extents[i]) dist = -obb.extents[i];

		pointOnOBB += dist * obb.axes[i];
	}

	return length(pointOnOBB + obb.center - sphere.position) < sphere.radius;
}
*/

bool intersect_point_frustum(const vec3& point, const Frustum& frustum)
{
	for(int i = 0; i < 6; i++)
	{
		float d = dot(frustum.planeNormals[i], point);
		if(d < frustum.planeDots[i])
			return false;
	}
	return true;
}

bool intersect_sphere_frustum(const vec3& center, float radius, const Frustum& frustum)
{
	for(int i = 0; i < 6; i++)
	{
		float d = dot(frustum.planeNormals[i], center);
		if(d + radius < frustum.planeDots[i])
			return false;
	}
	return true;
}

void cull_frustum_aabb_list(const Frustum& frustum, AABB* aabbList, int numAABBs, bool* stateList)
{
	// preemptively calculate absolute value of frustum plane normals and plane dot-products
	vec3 absPlaneNormals[6];
	for(int i = 0; i < 6; i++)
	{
		absPlaneNormals[i].x = fabs(frustum.planeNormals[i].x);
		absPlaneNormals[i].y = fabs(frustum.planeNormals[i].y);
		absPlaneNormals[i].z = fabs(frustum.planeNormals[i].z);
	}

	// intersect each AABB with the frustum
	for(int i = 0; i < numAABBs; i++)
	{
		// assume AABB is in frustum and reject it if it is found to be
		// outside any of the frustum planes
		bool result = true;
		for(int j = 0; j < 6; j++)
		{
			float d = dot(aabbList[i].center, frustum.planeNormals[j]);
			float r = dot(aabbList[i].extents, absPlaneNormals[j]);
			if(d + r < frustum.planeDots[j])
			{
				result = false;
				break;
			}
		}
		stateList[i] = result;
	}
}

void cull_frustum_obb_list(const Frustum& frustum, OBB* obbList, int numOBBs, bool* stateList)
{
	// intersect each OBB with the frustum
	for(int i = 0, n = numOBBs; i < n; ++i)
	{
		// assume OBB is in frustum and reject it if it is found to be
		// outside any of the frustum planes
		bool result = true;
		for(int j = 0; j < 6; ++j)
		{
			OBB obb = obbList[i];

			float x = (dot(obb.axes[0], -frustum.planeNormals[j]) >= 0.0f) ? obb.extents.x : -obb.extents.x;
			float y = (dot(obb.axes[1], -frustum.planeNormals[j]) >= 0.0f) ? obb.extents.y : -obb.extents.y;
			float z = (dot(obb.axes[2], -frustum.planeNormals[j]) >= 0.0f) ? obb.extents.z : -obb.extents.z;

			// Compute the half-diagonal vector (vector from OBB center to corner)
			// that points closest to the direction of the plane normal.
			vec3 diag = x * obb.axes[0] +
						y * obb.axes[1] +
						z * obb.axes[2];

			// get corner point of the OBB that lies "the most" inside the frustum
			vec3 corner = obb.center - diag;

			if(dot(corner, -frustum.planeNormals[j]) + frustum.planeDots[j] >= 0.0f)
			{
				result = false;
				break;
			}
		}
		stateList[i] = result;
	}
}

/*
bool intersect_obb_frustum(const vec3& center, const vec3& dimensions, const quaternion& orientation, const Frustum& frustum)
{
	// Do a SAT test along each frustum axis.
	vec3 xAxis = orientation * UNIT_X;
	vec3 yAxis = orientation * UNIT_Y;
	vec3 zAxis = orientation * UNIT_Z;

	for(int i = 0; i < 6; i++)
	{
		// Find the negative and positive far points of the OBB to the current frustum plane.
		float x = (dot(xAxis, -frustum.planes[i][0]) >= 0.0f) ? dimensions.x / 2.0f : -dimensions.x / 2.0f;
		float y = (dot(yAxis, -frustum.planes[i][0]) >= 0.0f) ? dimensions.y / 2.0f : -dimensions.y / 2.0f;
		float z = (dot(zAxis, -frustum.planes[i][0]) >= 0.0f) ? dimensions.z / 2.0f : -dimensions.z / 2.0f;

		// Compute the half-diagonal vector (vector from OBB center to corner) 
		// that points closest to the direction of the plane normal.
		const vec3 diag = x * xAxis + y * yAxis + z * zAxis;

		// nPoint = corner point of the OBB that lies "the most" inside the frustum 
		const vec3 nPoint = center - diag;

		if(dot(nPoint, -frustum.planes[i][0]) + dot(-frustum.planes[i][0], -frustum.planes[i][1]) >= 0.0f)
			return false;

		// If we would like to check whether the OBB is fully inside the frustum, need to compute
		// dot(pPoint, frustum.planes[i].normal) + frustum.planes[i].d. If it's < 0 for all planes, OBB is totally
		// inside the frustum and doesn't intersect any of the frustum planes.
	}
	return true;
}
*/

bool intersect_ray_triangle(vec3 orig, vec3 dir, vec3 vert0, vec3 vert1, vec3 vert2, vec3* intersect)
{
	vec3 edge1 = vert1 - vert0;
	vec3 edge2 = vert2 - vert0;

	/* begin calculating determinant - also used to calculate U*/
	vec3 pvec = cross(dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	float det = dot(edge1, pvec);

	/* calculate distance from vert0 to ray origin */
	vec3 tvec = orig - vert0;
	vec3 qvec = cross(tvec, edge1);
    
	float u, v;
	if(det > 1.0e-6f)
	{
		u = dot(tvec, pvec);
		if(u < 0.0 || u > det)
			return false;

		v = dot(dir, qvec);
		if(v < 0.0 || u + v > det)
			return false;
	}
	else if(det < -1.0e-6f)
	{
		u = dot(tvec, pvec);
		if(u > 0.0 || u < det)
			return false;

		v = dot(dir, qvec);
		if(v > 0.0 || u + v < det)
			return false;
	}
	else return false;  /* ray is parallel to the plane of the triangle */

	float inv_det = 1.0 / det;
	float t = dot(edge2, qvec) * inv_det;
	u *= inv_det;
	v *= inv_det;

	*intersect = orig + t * dir;
	return true;
}

struct Simplex
{
	vec3 points[4];
	int numPoints;
};

#define SAME_DIRECTION(point) dot((point), aO) > 0

bool enclose_line(Simplex* simplex, vec3* searchDirection)
{
	vec3 a = simplex->points[1];
	vec3 b = simplex->points[0];
	vec3 aO = -a;
	vec3 ab = b - a;
	if(SAME_DIRECTION(ab))
	{
		*searchDirection = triple_product(ab, aO, ab);
	}
	else
	{
		simplex->points[0] = a;
		simplex->numPoints = 1;

		*searchDirection = aO;
	}
	return false;
}

bool enclose_triangle(Simplex* simplex, vec3* searchDirection)
{
	vec3 a = simplex->points[2];
	vec3 b = simplex->points[1];
	vec3 c = simplex->points[0];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 abc = cross(ab, ac);

	vec3 aO = -a;
	vec3 acEdgeNormal = cross(abc, ac);
	// if origin is outside edge ac
	if(SAME_DIRECTION(acEdgeNormal))
	{
		// if origin is towards c, it cannot be outside ab
		if(SAME_DIRECTION(ac))
		{
			simplex->points[1] = a;
			simplex->numPoints = 2;
			
			*searchDirection = triple_product(ac, aO, ac);
		}
		else
		{
			// if outside ab, check whether ab or a is closest
			simplex->points[0] = b;
			simplex->points[1] = a;
			simplex->numPoints = 2;

			return enclose_line(simplex, searchDirection);
		}
	}
	// else, origin must be either in triangle or outside edge ab
	else
	{
		vec3 abEdgeNormal = cross(ab, abc);
		if(SAME_DIRECTION(abEdgeNormal))
		{
			// if outside ab, check whether ab or a is closest
			simplex->points[0] = b;
			simplex->points[1] = a;
			simplex->numPoints = 2;

			return enclose_line(simplex, searchDirection);
		}
		else
		{
			// if inside triangle, origin must be either above or below
			if(SAME_DIRECTION(abc))
			{
				*searchDirection = abc;
			}
			else
			{
				simplex->points[1] = c;
				simplex->points[0] = b;

				*searchDirection -abc;
			}
		}
	}
	return false;
}

bool enclose_tetrahedron(Simplex* simplex, vec3* searchDirection)
{
	vec3 a = simplex->points[3];
	vec3 b = simplex->points[2];
	vec3 c = simplex->points[1];
	vec3 d = simplex->points[0];

	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ad = d - a;
	
	vec3 abc = cross(ab, ac);
	vec3 acd = cross(ac, ad);
	vec3 adb = cross(ad, ab);

	vec3 aO = -a;

	if(SAME_DIRECTION(abc))
	{
		if(SAME_DIRECTION(acd))
		{
			if(SAME_DIRECTION(adb))
			{
				// eliminate b, c, d
				simplex->points[0] = a;
				simplex->numPoints = 1;

				*searchDirection = aO;
			}
			else
			{
				// eliminate b, d
				simplex->points[0] = c;
				simplex->points[1] = a;
				simplex->numPoints = 2;

				return enclose_line(simplex, searchDirection);
			}
		}
		else
		{
			if(SAME_DIRECTION(adb))
			{
				// eliminate c, d
				simplex->points[0] = b;
				simplex->points[1] = a;
				simplex->numPoints = 2;
				
				return enclose_line(simplex, searchDirection);
			}
			else
			{
				// eliminate d
				simplex->points[0] = c;
				simplex->points[1] = b;
				simplex->points[2] = a;
				simplex->numPoints = 3;

				return enclose_triangle(simplex, searchDirection);
			}
		}
	}
	else
	{
		if(SAME_DIRECTION(acd))
		{
			if(SAME_DIRECTION(adb))
			{
				// eliminate b, c
				simplex->points[1] = a;
				simplex->numPoints = 2;

				return enclose_line(simplex, searchDirection);
			}
			else
			{
				// eliminate b
				simplex->points[2] = a;
				simplex->numPoints = 3;

				return enclose_triangle(simplex, searchDirection);
			}
		}
		else
		{
			if(SAME_DIRECTION(adb))
			{
				// eliminate c
				simplex->points[1] = b;
				simplex->points[2] = a;
				simplex->numPoints = 3;

				return enclose_triangle(simplex, searchDirection);
			}
			else return true;
		}
	}
	return false;
}

bool simplex_contains_origin(Simplex* simplex, vec3* searchDirection)
{
	switch(simplex->numPoints)
	{
		case 2:
			return enclose_line(simplex, searchDirection);
		case 3:
			return enclose_triangle(simplex, searchDirection);
		case 4: default:
			return enclose_tetrahedron(simplex, searchDirection);
	}
}

vec3 get_furthest_in_direction(const ConvexRegion& region, vec3 direction)
{
	vec3 furthest = region.vertices[0];
	float maxDot = dot(direction, furthest);
	for(int i = 1; i < region.numVertices; i++)
	{
		float dist = dot(direction, region.vertices[i]);
		if(dist > maxDot)
		{
			maxDot = dist;
			furthest = region.vertices[i];
		}
	}
	return furthest;
}

vec3 find_support(const ConvexRegion& regionA, const ConvexRegion& regionB, vec3 searchDirection)
{
	return get_furthest_in_direction(regionA, searchDirection)
		 - get_furthest_in_direction(regionB, -searchDirection);
}

bool intersect_convex_regions(const ConvexRegion& regionA, const ConvexRegion& regionB)
{
	vec3 support = find_support(regionA, regionB, VEC3_ONE);

	Simplex simplex;
	simplex.points[0] = support;
	simplex.numPoints = 1;

	vec3 searchDirection = -support;
	do
	{
		vec3 a = find_support(regionA, regionB, searchDirection);
		if(dot(a, searchDirection) < 0)
			return false;
		simplex.points[simplex.numPoints++] = a;
	}
	while(!simplex_contains_origin(&simplex, &searchDirection));

	return true;
}

/*
void expanding_polytope(const ConvexRegion& regionA, const ConvexRegion& regionB, const Simplex& simplex)
{
	while(true)
	{
		Edge e = find_closest_edge(simplex);
		vec3 support = find_support(regionA, regionB, e.normal);

		float d = dot(support, e.normal);
		if(d - e.distance < 0.0001f)
		{
			normal = e.normal;
			depth = d;
		}
		else
		{
			simplex.points[simplex.numPoints++] = 
		}
	}
}
*/
