#ifndef GL_MODEL_H
#define GL_MODEL_H

#include "GLInfo.h"
#include "GLTexture.h"
#include "GLShader.h"

#include "../RenderPhase.h"

struct GLMaterial
{
	GLTexture texture;
	int startIndex;
	RenderPhase phase;
	vec4 color;
};

class GLModel
{
private:
	static const int MAX_MATERIALS = 8;

public:
	GLuint vertexArray;
	unsigned short numVertices, numIndices;

	GLMaterial materials[MAX_MATERIALS];
	int numMaterials;
	mat4x4 modelMatrix;
	bool isLoaded, isBillboarded, inBackground;

	GLModel();
	void LoadAsMesh(const String& fileName);
	void LoadAsQuad(const vec3& dimensions, const vec4& texCoord, bool isStatic = true);
	void Unload();

private:
	GLuint verticesV, elementsV;

	void SetDefaults();
	void BufferData(const vec4* vertices, const vec2* texcoords, const unsigned short* elements, bool isStatic = true);
};

#endif
