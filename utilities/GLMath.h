#ifndef GL_MATH_H
#define GL_MATH_H

// ----------------------------------------------------------------------------------------------------------------------------

class vec2
{
public:
	union
	{
		struct { float x, y; };
		struct { float s, t; };
		struct { float r, g; };
	};

	vec2() : x(0.0f), y(0.0f) {}
	vec2(float num) : x(num), y(num) {}
	vec2(float x, float y) : x(x), y(y) {}
	vec2(const vec2& u) : x(u.x), y(u.y) {}

	vec2& operator = (const vec2& u) { x = u.x; y = u.y; return *this; }
	vec2 operator - () const { return vec2(-x, -y); }
	vec2* operator & () { return (vec2*)this; }

	vec2& operator += (float num)  { x += num; y += num; return *this; }
	vec2& operator += (vec2 u)     { x += u.x; y += u.y; return *this; }
	vec2& operator -= (float num)  { x -= num; y -= num; return *this; }
	vec2& operator -= (vec2 u)     { x -= u.x; y -= u.y; return *this; }
	vec2& operator *= (float num)  { x *= num; y *= num; return *this; }
	vec2& operator *= (vec2 u)     { x *= u.x; y *= u.y; return *this; }
	vec2& operator /= (float num)  { x /= num; y /= num; return *this; }
	vec2& operator /= (vec2 u)     { x /= u.x; y /= u.y; return *this; }

	vec2 operator + (vec2 u) const  { return vec2(x + u.x, y + u.y); }
	vec2 operator - (vec2 u) const  { return vec2(x - u.x, y - u.y); }
	vec2 operator * (vec2 u) const  { return vec2(x * u.x, y * u.y); }
	vec2 operator / (vec2 u) const  { return vec2(x / u.x, y / u.y); }

	friend vec2 operator + (vec2 u, float num)  { return vec2(u.x + num, u.y + num); }
	friend vec2 operator + (float num, vec2 u)  { return vec2(num + u.x, num + u.y); }
	friend vec2 operator - (vec2 u, float num)  { return vec2(u.x - num, u.y - num); }
	friend vec2 operator - (float num, vec2 u)  { return vec2(num - u.x, num - u.y); }
	friend vec2 operator * (vec2 u, float num)  { return vec2(u.x * num, u.y * num); }
	friend vec2 operator * (float num, vec2 u)  { return vec2(num * u.x, num * u.y); }
	friend vec2 operator / (vec2 u, float num)  { return vec2(u.x / num, u.y / num); }
	friend vec2 operator / (float num, vec2 u)  { return vec2(num / u.x, num / u.y); }
};

// ----------------------------------------------------------------------------------------------------------------------------

class vec3
{
public:
	union
	{
		struct { float x, y, z; };
		struct { float s, t, p; };
		struct { float r, g, b; };
	};

	vec3() : x(0.0f), y(0.0f), z(0.0f) {}
	vec3(float num) : x(num), y(num), z(num) {}
	vec3(float x, float y, float z) : x(x), y(y), z(z) {}
	vec3(const vec2& u, float z) : x(u.x), y(u.y), z(z) {}
	vec3(const vec3& u) : x(u.x), y(u.y), z(u.z) {}

	vec3& operator = (vec3 u) { x = u.x; y = u.y; z = u.z; return *this; }
	vec3 operator - () const { return vec3(-x, -y, -z); }
	vec3* operator & () { return (vec3*)this; }
	operator vec2 () { return *(vec2*)this; }

	vec3& operator += (float num)  { x += num; y += num; z += num; return *this; }
	vec3& operator += (vec3 u)     { x += u.x; y += u.y; z += u.z; return *this; }
	vec3& operator -= (float num)  { x -= num; y -= num; z -= num; return *this; }
	vec3& operator -= (vec3 u)     { x -= u.x; y -= u.y; z -= u.z; return *this; }
	vec3& operator *= (float num)  { x *= num; y *= num; z *= num; return *this; }
	vec3& operator *= (vec3 u)     { x *= u.x; y *= u.y; z *= u.z; return *this; }
	vec3& operator /= (float num)  { x /= num; y /= num; z /= num; return *this; }
	vec3& operator /= (vec3 u)     { x /= u.x; y /= u.y; z /= u.z; return *this; }

	vec3 operator + (vec3 u) const   { return vec3(x + u.x, y + u.y, z + u.z); }
	vec3 operator - (vec3 u) const   { return vec3(x - u.x, y - u.y, z - u.z); }
	vec3 operator * (vec3 u) const   { return vec3(x * u.x, y * u.y, z * u.z); }
	vec3 operator / (vec3 u) const   { return vec3(x / u.x, y / u.y, z / u.z); }
	bool operator == (vec3 u) const  { return x == u.x && y == u.y && z == u.z; }
	bool operator != (vec3 u) const  { return x != u.x || y != u.y || z != u.z; }

	friend vec3 operator + (vec3 u, float num)  { return vec3(u.x + num, u.y + num, u.z + num); }
	friend vec3 operator + (float num, vec3 u)  { return vec3(num + u.x, num + u.y, num + u.z); }
	friend vec3 operator - (vec3 u, float num)  { return vec3(u.x - num, u.y - num, u.z - num); }
	friend vec3 operator - (float num, vec3 u)  { return vec3(num - u.x, num - u.y, num - u.z); }
	friend vec3 operator * (vec3 u, float num)  { return vec3(u.x * num, u.y * num, u.z * num); }
	friend vec3 operator * (float num, vec3 u)  { return vec3(num * u.x, num * u.y, num * u.z); }
	friend vec3 operator / (vec3 u, float num)  { return vec3(u.x / num, u.y / num, u.z / num); }
	friend vec3 operator / (float num, vec3 u)  { return vec3(num / u.x, num / u.y, num / u.z); }
};

// ----------------------------------------------------------------------------------------------------------------------------

class vec4
{
public:
	union
	{
		struct { float x, y, z, w; };
		struct { float s, t, p, q; };
		struct { float r, g, b, a; };
	};

	vec4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	vec4(float num) : x(num), y(num), z(num), w(num) {}
	vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	vec4(const vec2& u, float z, float w) : x(u.x), y(u.y), z(z), w(w) {}
	vec4(const vec3& u, float w) : x(u.x), y(u.y), z(u.z), w(w) {}
	vec4(const vec4& u) : x(u.x), y(u.y), z(u.z), w(u.w) {}

	vec4& operator = (vec4 u) { x = u.x; y = u.y; z = u.z; w = u.w; return *this; }
	vec4 operator - () const { return vec4(-x, -y, -z, -w); }
	vec4* operator & () { return (vec4*)this; }
	operator vec2 () { return *(vec2*)this; }
	operator vec3 () { return *(vec3*)this; }

	vec4& operator += (float num)  { x += num; y += num; z += num; w += num; return *this; }
	vec4& operator += (vec4 u)     { x += u.x; y += u.y; z += u.z; w += u.w; return *this; }
	vec4& operator -= (float num)  { x -= num; y -= num; z -= num; w -= num; return *this; }
	vec4& operator -= (vec4 u)     { x -= u.x; y -= u.y; z -= u.z; w -= u.w; return *this; }
	vec4& operator *= (float num)  { x *= num; y *= num; z *= num; w *= num; return *this; }
	vec4& operator *= (vec4 u)     { x *= u.x; y *= u.y; z *= u.z; w *= u.w; return *this; }
	vec4& operator /= (float num)  { x /= num; y /= num; z /= num; w /= num; return *this; }
	vec4& operator /= (vec4 u)     { x /= u.x; y /= u.y; z /= u.z; w /= u.w; return *this; }

	vec4 operator + (vec4 u) const  { return vec4(x + u.x, y + u.y, z + u.z, w + u.w); }
	vec4 operator - (vec4 u) const  { return vec4(x - u.x, y - u.y, z - u.z, w - u.w); }
	vec4 operator * (vec4 u) const  { return vec4(x * u.x, y * u.y, z * u.z, w * u.w); }
	vec4 operator / (vec4 u) const  { return vec4(x / u.x, y / u.y, z / u.z, w / u.w); }

	friend vec4 operator + (vec4 u, float num)  { return vec4(u.x + num, u.y + num, u.z + num, u.w + num); }
	friend vec4 operator + (float num, vec4 u)  { return vec4(num + u.x, num + u.y, num + u.z, num + u.w); }
	friend vec4 operator - (vec4 u, float num)  { return vec4(u.x - num, u.y - num, u.z - num, u.w - num); }
	friend vec4 operator - (float num, vec4 u)  { return vec4(num - u.x, num - u.y, num - u.z, num - u.w); }
	friend vec4 operator * (vec4 u, float num)  { return vec4(u.x * num, u.y * num, u.z * num, u.w * num); }
	friend vec4 operator * (float num, vec4 u)  { return vec4(num * u.x, num * u.y, num * u.z, num * u.w); }
	friend vec4 operator / (vec4 u, float num)  { return vec4(u.x / num, u.y / num, u.z / num, u.w / num); }
	friend vec4 operator / (float num, vec4 u)  { return vec4(num / u.x, num / u.y, num / u.z, num / u.w); }
};

// ----------------------------------------------------------------------------------------------------------------------------

class quaternion;

class mat4x4
{
public:
	float M[16];

	mat4x4();
	mat4x4(const mat4x4& matrix);
	mat4x4(quaternion quat);

	mat4x4& operator = (const mat4x4& matrix);
	float& operator [] (int index) { return M[index]; }
	const float& operator [] (int index) const { return M[index]; }

	bool operator == (const mat4x4& matrix) const;
	bool operator != (const mat4x4& matrix) const;
	mat4x4 operator * (const mat4x4& matrix) const;
	vec4 operator * (vec4 u) const;
	vec3 operator * (vec3 u) const { return *this * vec4(u, 1.0f); }
	vec2 operator * (vec2 u) const { return *this * vec4(u, 0.0f, 1.0f); }
};

// ----------------------------------------------------------------------------------------------------------------------------

class quaternion
{
public:
	struct { float x, y, z, w; };

	quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	quaternion operator * (quaternion rq) const;
	quaternion& operator *= (quaternion rq);
	vec3 operator * (vec3 vec) const;
};

// ----------------------------------------------------------------------------------------------------------------------------

#define VEC2_ZERO vec2(0.0f, 0.0f)

#define VEC3_ZERO vec3(0.0f, 0.0f, 0.0f)
#define VEC3_ONE vec3(1.0f, 1.0f, 1.0f)
#define UNIT_X vec3(1.0f, 0.0f, 0.0f)
#define UNIT_Y vec3(0.0f, 1.0f, 0.0f)
#define UNIT_Z vec3(0.0f, 0.0f, 1.0f)

#define VEC4_ZERO vec4(0.0f, 0.0f, 0.0f, 0.0f)
#define VEC4_ONE vec4(1.0f, 1.0f, 1.0f, 1.0f)

#define QUAT_I quaternion(0.0f, 0.0f, 0.0f, 1.0f)

#define MAT_I mat4x4()

// ----------------------------------------------------------------------------------------------------------------------------

float dot(vec2 u, vec2 v);
float length(vec2 u);
vec2 normalize(vec2 u);
vec2 reflect(vec2 i, vec2 n);
vec2 refract(vec2 i, vec2 n, float eta);
vec2 rotate(vec2 u, float angle);

// ----------------------------------------------------------------------------------------------------------------------------

vec3 cross(vec3 u, vec3 v);
float dot(vec3 u, vec3 v);
vec3 triple_product(vec3 u, vec3 v, vec3 w);
float length(vec3 u);
vec3 mix(vec3 u, vec3 v, float a);
vec3 normalize(vec3 u);
vec3 reflect(vec3 i, vec3 n);
vec3 refract(vec3 i, vec3 n, float eta);
vec3 rotate(vec3 u, float angle, vec3 axis);

// ----------------------------------------------------------------------------------------------------------------------------

quaternion normalize(quaternion quat);
quaternion quat_from_axis_angle(vec3 v, float angle);
quaternion quat_from_matrix(mat4x4& m);
quaternion invert_quat(quaternion quat);
quaternion slerp(quaternion from, quaternion to, float t);
quaternion rotation_to(vec3 from, vec3 to, vec3 fallbackAxis = VEC3_ZERO);
quaternion quat_from_euler(float roll, float pitch, float yaw);

// ----------------------------------------------------------------------------------------------------------------------------

mat4x4 bias_matrix();
mat4x4 bias_matrix_inverse();
mat4x4 look_at_matrix(vec3 eyePosition, vec3 lookAt, vec3 upVector);
mat4x4 view_matrix(vec3 x, vec3 y, vec3 z, vec3 position);
mat4x4 view_matrix_inverse(const mat4x4& m);
mat4x4 orthogonal_projection_matrix(float left, float right, float bottom, float top, float n, float f);
mat4x4 perspective_projection_matrix(float fovy, float x, float y, float n, float f);
mat4x4 perspective_projection_matrix_inverse(const mat4x4& m);
mat4x4 rotation_matrix(float angle, vec3 u);
mat4x4 scale_matrix(float x, float y, float z);
mat4x4 translation_matrix(float x, float y, float z);
mat4x4 inverse_matrix(const mat4x4& m);
mat4x4 transpose_matrix(const mat4x4& m);

mat4x4 view_orientation(const mat4x4& m);

#endif
