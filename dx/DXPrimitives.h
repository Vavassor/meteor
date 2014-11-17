#ifndef DX_PRIMITIVES_H
#define DX_PRIMITIVES_H

#include "../utilities/GLMath.h"

namespace DXPrimitives
{
	void Initialize();
	void Terminate();

	void DrawTris(const vec4 vertices[], const vec2 texCoords[], int numVertices);
	void DrawQuads(const vec4 vertices[], const vec2 texCoords[], int numQuadVertices);
	void DrawLines(const vec4 vertices[], int numVertices);
	void DrawLineCircle(vec3 position, float radius, vec3 axisOfRotation = UNIT_Z);
	void DrawLineSphere(vec3 position, float radius);
	void DrawLineBox(vec3 center, vec3 dimensions, quaternion orientation = QUAT_I);
}

#endif
