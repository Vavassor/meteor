#ifndef GL_PRIMITIVES_H
#define GL_PRIMITIVES_H

#include "../utilities/GLMath.h"

namespace GLPrimitives
{
	void Initialize();
	void Terminate();
	void Draw();

	void AddTris(const vec4 vertices[], const vec2 texCoords[], int numVertices);
	void AddQuads(const vec4 vertices[], const vec2 texCoords[], int numQuadVertices);
	void AddLines(const vec4 vertices[], int numVertices);
	void AddLineCircle(vec3 position, float radius, vec3 axisOfRotation = UNIT_Z);
	void AddLineSphere(vec3 position, float radius);
	void AddLineBox(vec3 center, vec3 dimensions, quaternion orientation = QUAT_I);
}

#endif
