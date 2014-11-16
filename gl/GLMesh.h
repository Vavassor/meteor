#ifndef GL_MESH_H
#define GL_MESH_H

#include "GLInfo.h"

#include "utilities/GLMath.h"
#include "utilities/DataTypes.h"

struct ObjectBlock
{
	mat4x4 modelViewProjection;
};

struct MaterialBlock
{
	vec4 color;
};

class GLMesh
{
public:
	ObjectBlock objectBlock;
	MaterialBlock materialBlock;

	union
	{
		uint64_t sortingKey;
		struct
		{
			uint64_t material		: 30,
					 depth			: 24,
					 sortingLayer	: 5,
					 phase			: 3,
					 viewportLayer	: 2;
		};
	};

	GLuint vertexArray;
	GLuint startIndex;
	GLuint numIndices;

	GLuint textureID;
	mat4x4 model;

	GLMesh();
	void Draw() const;
};

#endif
