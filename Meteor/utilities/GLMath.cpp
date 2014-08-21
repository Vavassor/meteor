#include "GLMath.h"

#include "Maths.h"

#include <math.h>

// ----------------------------------------------------------------------------------------------------------------------------

mat4x4::mat4x4()
{
	M[0] = 1.0f; M[4] = 0.0f; M[8] = 0.0f; M[12] = 0.0f;
	M[1] = 0.0f; M[5] = 1.0f; M[9] = 0.0f; M[13] = 0.0f;
	M[2] = 0.0f; M[6] = 0.0f; M[10] = 1.0f; M[14] = 0.0f;
	M[3] = 0.0f; M[7] = 0.0f; M[11] = 0.0f; M[15] = 1.0f;
}

mat4x4::mat4x4(const mat4x4& matrix)
{
	for(int i = 0; i < 16; i++)
		M[i] = matrix[i];
}

mat4x4::mat4x4(const quaternion& quat)
{
	float x2 = quat.x * quat.x;
	float y2 = quat.y * quat.y;
	float z2 = quat.z * quat.z;
	float xy = quat.x * quat.y;
	float xz = quat.x * quat.z;
	float yz = quat.y * quat.z;
	float wx = quat.w * quat.x;
	float wy = quat.w * quat.y;
	float wz = quat.w * quat.z;
 
	M[0] = 1.0f - 2.0f * (y2 + z2);
	M[4] = 2.0f * (xy - wz);
	M[8] = 2.0f * (xz + wy);
	M[12] = 0.0f;

	M[1] = 2.0f * (xy + wz);
	M[5] = 1.0f - 2.0f * (x2 + z2);
	M[9] = 2.0f * (yz - wx);
	M[13] = 0.0f;

	M[2] = 2.0f * (xz - wy);
	M[6] = 2.0f * (yz + wx);
	M[10] = 1.0f - 2.0f * (x2 + y2);
	M[14] = 0.0f;

	M[3] = 0.0f;
	M[7] = 0.0f;
	M[11] = 0.0f;
	M[15] = 1.0f;
}

mat4x4& mat4x4::operator = (const mat4x4& matrix)
{
	for(int i = 0; i < 16; i++)
		M[i] = matrix[i];
	return *this;
}

bool mat4x4::operator == (const mat4x4& matrix) const
{
	for(int i = 0; i < 16; i++)
	{
		if(M[i] != matrix[i]) return false;
	}
	return true;
}

bool mat4x4::operator != (const mat4x4& matrix) const
{
	for(int i = 0; i < 16; i++)
	{
		if(M[i] != matrix[i]) return true;
	}
	return false;
}

mat4x4 mat4x4::operator * (const mat4x4& matrix) const
{
	mat4x4 out;

	out[0] = M[0] * matrix[0] + M[4] * matrix[1] + M[8] * matrix[2] + M[12] * matrix[3];
	out[1] = M[1] * matrix[0] + M[5] * matrix[1] + M[9] * matrix[2] + M[13] * matrix[3];
	out[2] = M[2] * matrix[0] + M[6] * matrix[1] + M[10] * matrix[2] + M[14] * matrix[3];
	out[3] = M[3] * matrix[0] + M[7] * matrix[1] + M[11] * matrix[2] + M[15] * matrix[3];

	out[4] = M[0] * matrix[4] + M[4] * matrix[5] + M[8] * matrix[6] + M[12] * matrix[7];
	out[5] = M[1] * matrix[4] + M[5] * matrix[5] + M[9] * matrix[6] + M[13] * matrix[7];
	out[6] = M[2] * matrix[4] + M[6] * matrix[5] + M[10] * matrix[6] + M[14] * matrix[7];
	out[7] = M[3] * matrix[4] + M[7] * matrix[5] + M[11] * matrix[6] + M[15] * matrix[7];

	out[8] = M[0] * matrix[8] + M[4] * matrix[9] + M[8] * matrix[10] + M[12] * matrix[11];
	out[9] = M[1] * matrix[8] + M[5] * matrix[9] + M[9] * matrix[10] + M[13] * matrix[11];
	out[10] = M[2] * matrix[8] + M[6] * matrix[9] + M[10] * matrix[10] + M[14] * matrix[11];
	out[11] = M[3] * matrix[8] + M[7] * matrix[9] + M[11] * matrix[10] + M[15] * matrix[11];

	out[12] = M[0] * matrix[12] + M[4] * matrix[13] + M[8] * matrix[14] + M[12] * matrix[15];
	out[13] = M[1] * matrix[12] + M[5] * matrix[13] + M[9] * matrix[14] + M[13] * matrix[15];
	out[14] = M[2] * matrix[12] + M[6] * matrix[13] + M[10] * matrix[14] + M[14] * matrix[15];
	out[15] = M[3] * matrix[12] + M[7] * matrix[13] + M[11] * matrix[14] + M[15] * matrix[15];

	return out;
}

vec4 mat4x4::operator * (const vec4& u) const
{
	return vec4(M[0] * u.x + M[4] * u.y + M[8] * u.z + M[12] * u.w,
				M[1] * u.x + M[5] * u.y + M[9] * u.z + M[13] * u.w,
				M[2] * u.x + M[6] * u.y + M[10] * u.z + M[14] * u.w,
				M[3] * u.x + M[7] * u.y + M[11] * u.z + M[15] * u.w);
}

// ----------------------------------------------------------------------------------------------------------------------------

quaternion quaternion::operator * (const quaternion &rq) const
{
	return quaternion(w * rq.x + x * rq.w + y * rq.z - z * rq.y,
					  w * rq.y + y * rq.w + z * rq.x - x * rq.z,
					  w * rq.z + z * rq.w + x * rq.y - y * rq.x,
					  w * rq.w - x * rq.x - y * rq.y - z * rq.z);
}

quaternion& quaternion::operator *= (const quaternion &rq)
{
	*this = *this * rq;
	return *this;
}

vec3 quaternion::operator * (const vec3 &v) const
{
	vec3 u(x, y, z);
	float s = w;
	return	2.0f * dot(u, v) * u
		+ (s * s - dot(u, u)) * v
		+ 2.0f * s * cross(u, v);
}

// ----------------------------------------------------------------------------------------------------------------------------

float dot(const vec2& u, const vec2& v)
{
	return u.x * v.x + u.y * v.y;
}

float length(const vec2& u)
{
	return sqrt(u.x * u.x + u.y * u.y);
}

vec2 normalize(const vec2& u)
{
	return u / sqrt(u.x * u.x + u.y * u.y);
}

vec2 reflect(const vec2& i, const vec2& n)
{
	return i - 2.0f * dot(n, i) * n;
}

vec2 refract(const vec2& i, const vec2& n, float eta)
{
	float ndoti = dot(n, i);
	float k = 1.0f - eta * eta * (1.0f - ndoti * ndoti);

	vec2 r;
	if(k >= 0.0f) r = eta * i - n * (eta * ndoti + sqrt(k));

	return r;
}

vec2 rotate(const vec2& u, float angle)
{
	float cs = cos(angle);
	float sn = sin(angle);
	return vec2(u.x * cs - u.y * sn,
				u.x * sn + u.y * cs);
}

// ----------------------------------------------------------------------------------------------------------------------------

vec3 cross(const vec3& u, const vec3& v)
{
	return vec3(u.y * v.z - u.z * v.y,
				u.z * v.x - u.x * v.z,
				u.x * v.y - u.y * v.x);
}

float dot(const vec3& u, const vec3& v)
{
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

vec3 triple_product(const vec3& u, const vec3& v, const vec3& w)
{
	return -u * dot(v, w) + v * dot(u, w);
}

float length(const vec3& u)
{
	return sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
}

vec3 mix(const vec3& u, const vec3& v, float a)
{
	return u * (1.0f - a) + v * a;
}

vec3 normalize(const vec3& u)
{
	return u / sqrt(u.x * u.x + u.y * u.y + u.z * u.z);
}

vec3 reflect(const vec3& i, const vec3& n)
{
	return i - 2.0f * dot(n, i) * n;
}

vec3 refract(const vec3& i, const vec3& n, float eta)
{
	float ndoti = dot(n, i);
	float k = 1.0f - eta * eta * (1.0f - ndoti * ndoti);

	vec3 r;
	if(k >= 0.0f)
		r = eta * i - n * (eta * ndoti + sqrt(k));

	return r;
}

vec3 rotate(const vec3& u, float angle, const vec3& axis)
{
	return quat_from_axis_angle(axis, angle) * u;
}

// ----------------------------------------------------------------------------------------------------------------------------

quaternion normalize(const quaternion& q)
{
	float mag2 = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;

	quaternion out = q;
	if (fabs(mag2) > 0.00001f && fabs(mag2 - 1.0f) > 0.00001f)
	{
		float mag = sqrt(mag2);
		out.w /= mag;
		out.x /= mag;
		out.y /= mag;
		out.z /= mag;
	}
	return out;
}

quaternion quat_from_axis_angle(const vec3& v, float angle)
{
	angle *= 0.5f;
	vec3 vn = normalize(v);
	float sinAngle = sin(angle);

	return quaternion(vn.x * sinAngle,
					  vn.y * sinAngle,
					  vn.z * sinAngle,
					  cos(angle));
}

quaternion quat_from_matrix(mat4x4& m)
{
	quaternion quat;
	float trace = m[0] + m[5] + m[10];
	if(trace > 0)
	{
		float s = 0.5f / sqrtf(trace+ 1.0f);
		quat.w = 0.25f / s;
		quat.x = (m[9] - m[6]) * s;
		quat.y = (m[2] - m[8]) * s;
		quat.z = (m[4] - m[1]) * s;
	} 
	else 
	{
		if (m[0] > m[5] && m[0] > m[10])
		{
			float s = 2.0f * sqrtf(1.0f + m[0] - m[5] - m[10]);
			quat.w = (m[9] - m[6]) / s;
			quat.x = 0.25f * s;
			quat.y = (m[1] + m[4]) / s;
			quat.z = (m[2] + m[8]) / s;
		} 
		else if (m[5] > m[10]) 
		{
			float s = 2.0f * sqrtf(1.0f + m[5] - m[0] - m[10]);
			quat.w = (m[2] - m[8]) / s;
			quat.x = (m[1] + m[4]) / s;
			quat.y = 0.25f * s;
			quat.z = (m[6] + m[9]) / s;
		} 
		else 
		{
			float s = 2.0f * sqrtf(1.0f + m[10] - m[0] - m[5]);
			quat.w = (m[4] - m[1] ) / s;
			quat.x = (m[2] + m[8] ) / s;
			quat.y = (m[6] + m[9] ) / s;
			quat.z = 0.25f * s;
		}
	}
	return quat;
}

quaternion invert_quat(const quaternion& quat)
{
	float dot = quat.x * quat.x + quat.y * quat.y + quat.z * quat.z + quat.w * quat.w;

	return quaternion(-quat.x / dot,
					  -quat.y / dot,
					  -quat.z / dot,
					   quat.w / dot);
}

quaternion slerp(const quaternion& from, const quaternion& to, float t)
{
	// calc cosine
	float to1[4];
	float cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;

	// adjust signs (if necessary)
	if (cosom < 0.0)
	{ 
		cosom = -cosom;
		to1[0] = -to.x;
		to1[1] = -to.y;
		to1[2] = -to.z;
		to1[3] = -to.w;
	} 
	else  
	{
		to1[0] = to.x;
		to1[1] = to.y;
		to1[2] = to.z;
		to1[3] = to.w;
	}

	// calculate coefficients
	double scale0, scale1;
	if(1.0 - cosom > 0.0005f)
	{
		// standard case (slerp)
		double omega = acos(cosom);
		double sinom = sin(omega);
		scale0 = sin((1.0 - t) * omega) / sinom;
		scale1 = sin(t * omega) / sinom;
	}
	else
	{
		// "from" and "to" quaternions are very close
		//  ... so we can do a linear interpolation
		scale0 = 1.0 - t;
		scale1 = t;
	}
	
	return quaternion(scale0 * from.x + scale1 * to1[0],
					  scale0 * from.y + scale1 * to1[1],
					  scale0 * from.z + scale1 * to1[2],
					  scale0 * from.w + scale1 * to1[3]);
}

quaternion rotation_to(const vec3& from, const vec3& to, const vec3& fallbackAxis)
{
	vec3 v0 = normalize(from);
	vec3 v1 = normalize(to);

	float d = dot(v0, v1);
	// If dot == 1, vectors are the same
	if (d >= 1.0f) return QUAT_I;

	quaternion q;
	if(d < 1.0e-6f - 1.0f)
	{
		if(fallbackAxis != VEC3_ZERO)
		{
			// rotate 180 degrees about the fallback axis
			q = quat_from_axis_angle(fallbackAxis, M_PI);
		}
		else
		{
			// Generate an axis
			vec3 axis = cross(UNIT_X, v0);
			if (length(axis) < 1.0e-6f) // pick another if colinear
				axis = cross(UNIT_Y, v0);
			axis = normalize(axis);
			q = quat_from_axis_angle(axis, M_PI);
		}
	}
	else
	{
		float s = sqrt((1 + d) * 2);
		float invs = 1 / s;

		vec3 c = cross(v0, v1);

		q.x = c.x * invs;
		q.y = c.y * invs;
		q.z = c.z * invs;
		q.w = s * 0.5f;
		q = normalize(q);
	}
	return q;
}

quaternion quat_from_euler(float roll, float pitch, float yaw)
{
	// calculate trig identities
	float cr = cos(roll / 2);
	float cp = cos(pitch / 2);
	float cy = cos(yaw / 2);
	float sr = sin(roll / 2);
	float sp = sin(pitch / 2);
	float sy = sin(yaw / 2);

	float cpcy = cp * cy;
	float spsy = sp * sy;

	return quaternion(sr * cpcy - cr * spsy,
					  cr * sp * cy + sr * cp * sy,
					  cr * cp * sy - sr * sp * cy,
					  cr * cpcy + sr * spsy);
}

// ----------------------------------------------------------------------------------------------------------------------------

mat4x4 bias_matrix()
{
	mat4x4 B;

	B[0] = 0.5f; B[4] = 0.0f; B[8] = 0.0f; B[12] = 0.5f;
	B[1] = 0.0f; B[5] = 0.5f; B[9] = 0.0f; B[13] = 0.5f;
	B[2] = 0.0f; B[6] = 0.0f; B[10] = 0.5f; B[14] = 0.5f;
	B[3] = 0.0f; B[7] = 0.0f; B[11] = 0.0f; B[15] = 1.0f;

	return B;
}

mat4x4 bias_matrix_inverse()
{
	mat4x4 BI;

	BI[0] = 2.0f; BI[4] = 0.0f; BI[8] = 0.0f; BI[12] = -1.0f;
	BI[1] = 0.0f; BI[5] = 2.0f; BI[9] = 0.0f; BI[13] = -1.0f;
	BI[2] = 0.0f; BI[6] = 0.0f; BI[10] = 2.0f; BI[14] = -1.0f;
	BI[3] = 0.0f; BI[7] = 0.0f; BI[11] = 0.0f; BI[15] = 1.0f;

	return BI;
}

mat4x4 look_at_matrix(const vec3& eyePosition, const vec3& lookAt, const vec3& upVector)
{
	vec3 e = eyePosition;
	vec3 up = normalize(upVector);

	// forward vector
	vec3 forward = normalize(lookAt - e);

	// side vector
	vec3 side = cross(forward, up);
	side = normalize(side);

	up = normalize(cross(side, forward));

	mat4x4 look;

	look[0] = side.x;
	look[4] = side.y;
	look[8] = side.z;

	look[1] = up.x;
	look[5] = up.y;
	look[9] = up.z;

	look[2] = -forward.x;
	look[6] = -forward.y;
	look[10] = -forward.z;

	look[12] = -e.x;
	look[13] = -e.y;
	look[14] = -e.z;

	return look;
}

mat4x4 view_matrix(const vec3 &x, const vec3 &y, const vec3 &z, const vec3 &position)
{
	mat4x4 v;

	v[0] = x.x;
	v[1] = y.x;
	v[2] = z.x;
	v[3] = 0.0f;

	v[4] = x.y;
	v[5] = y.y;
	v[6] = z.y;
	v[7] = 0.0f;

	v[8] = x.z;
	v[9] = y.z;
	v[10] = z.z;
	v[11] = 0.0f;

	v[12] = -dot(x, position);
	v[13] = -dot(y, position);
	v[14] = -dot(z, position);
	v[15] = 1.0f;

	return v;
}

mat4x4 view_matrix_inverse(const mat4x4 &m)
{
	mat4x4 inverse;

	inverse[0] = m[0];
	inverse[1] = m[4];
	inverse[2] = m[8];
	inverse[3] = 0.0f;

	inverse[4] = m[1];
	inverse[5] = m[5];
	inverse[6] = m[9];
	inverse[7] = 0.0f;

	inverse[8] = m[2];
	inverse[9] = m[6];
	inverse[10] = m[10];
	inverse[11] = 0.0f;

	inverse[12] = -(inverse[0] * m[12] + inverse[4] * m[13] + inverse[8] * m[14]);
	inverse[13] = -(inverse[1] * m[12] + inverse[5] * m[13] + inverse[9] * m[14]);
	inverse[14] = -(inverse[2] * m[12] + inverse[6] * m[13] + inverse[10] * m[14]);
	inverse[15] = 1.0f;

	return inverse;
}

mat4x4 orthogonal_projection_matrix(float left, float right, float bottom, float top, float near, float far)
{
	mat4x4 matrix;

	matrix[0] = 2.0f / (right - left);
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = 2.0f / (top - bottom);
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = -2.0f / (far - near);
	matrix[11] = 0.0f;

	matrix[12] = -(right + left) / (right - left);
	matrix[13] = -(top + bottom) / (top - bottom);
	matrix[14] = -(far + near) / (far - near);
	matrix[15] = 1.0f;

	return matrix;
}

mat4x4 perspective_projection_matrix(float fovy, float x, float y, float near, float far)
{
	float coty = 1.0f / tan(fovy * M_PI / 360.0f);
	float aspect = x / (y > 0.0f ? y : 1.0f);

	mat4x4 matrix;

	matrix[0] = coty / aspect;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = coty;
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = (near + far) / (near - far);
	matrix[11] = -1.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 2.0f * near * far / (near - far);
	matrix[15] = 0.0f;

	return matrix;
}

mat4x4 perspective_projection_matrix_inverse(const mat4x4& m)
{
	mat4x4 inverse;

	inverse[0] = 1.0f / m[0];
	inverse[1] = 0.0f;
	inverse[2] = 0.0f;
	inverse[3] = 0.0f;

	inverse[4] = 0.0f;
	inverse[5] = 1.0f / m[5];
	inverse[6] = 0.0f;
	inverse[7] = 0.0f;

	inverse[8] = 0.0f;
	inverse[9] = 0.0f;
	inverse[10] = 0.0f;
	inverse[11] = 1.0f / m[14];

	inverse[12] = 0.0f;
	inverse[13] = 0.0f;
	inverse[14] = 1.0f / m[11];
	inverse[15] = -m[10] / (m[11] * m[14]);

	return inverse;
}

mat4x4 rotation_matrix(float angle, const vec3& u)
{
	angle = angle / 180.0f * (float)M_PI;

	vec3 v = normalize(u);

	float c = 1.0f - cos(angle);
	float s = sin(angle);

	mat4x4 matrix;

	matrix[0] = 1.0f + c * (v.x * v.x - 1.0f);
	matrix[1] = c * v.x * v.y + v.z * s;
	matrix[2] = c * v.x * v.z - v.y * s;
	matrix[3] = 0.0f;

	matrix[4] = c * v.x * v.y - v.z * s;
	matrix[5] = 1.0f + c * (v.y * v.y - 1.0f);
	matrix[6] = c * v.y * v.z + v.x * s;
	matrix[7] = 0.0f;

	matrix[8] = c * v.x * v.z + v.y * s;
	matrix[9] = c * v.y * v.z - v.x * s;
	matrix[10] = 1.0f + c * (v.z * v.z - 1.0f);
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	return matrix;
}

mat4x4 scale_matrix(float x, float y, float z)
{
	mat4x4 matrix;

	matrix[0] = x;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = y;
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = z;
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;

	return matrix;
}

mat4x4 translation_matrix(float x, float y, float z)
{
	mat4x4 matrix;

	matrix[0] = 1.0f;
	matrix[1] = 0.0f;
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = 0.0f;
	matrix[5] = 1.0f;
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = 1.0f;
	matrix[11] = 0.0f;

	matrix[12] = x;
	matrix[13] = y;
	matrix[14] = z;
	matrix[15] = 1.0f;

	return matrix;
}

mat4x4 inverse_matrix(const mat4x4& m)
{
    mat4x4 inv;
    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    float det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if(det == 0.0f) return m;

    det = 1.0f / det;

    for(int i = 0; i < 16; i++)
        inv[i] *= det;

    return inv;
}

mat4x4 transpose_matrix(const mat4x4& m)
{
	mat4x4 t;

	t[0] = m[0];
	t[1] = m[4];
	t[2] = m[8];
	t[3] = m[12];

	t[4] = m[1];
	t[5] = m[5];
	t[6] = m[9];
	t[7] = m[13];

	t[8] = m[2];
	t[9] = m[6];
	t[10] = m[10];
	t[11] = m[14];

	t[12] = m[3];
	t[13] = m[7];
	t[14] = m[11];
	t[15] = m[15];

	return t;
}

mat4x4 view_orientation(const mat4x4& m)
{
	mat4x4 o;

	o[0] = m[0];
	o[1] = m[1];
	o[2] = m[2];

	o[4] = m[4];
	o[5] = m[5];
	o[6] = m[6];

	o[8] = m[8];
	o[9] = m[9];
	o[10] = m[10];

	return inverse_matrix(o);
}
