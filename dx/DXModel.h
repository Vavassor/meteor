#ifndef DX_MODEL_H
#define DX_MODEL_H

#include "DXShader.h"

#include "../utilities/GLMath.h"
#include "../utilities/String.h"

#include "../RenderPhase.h"

enum ModelUsage
{
	USAGE_STATIC,
	USAGE_DYNAMIC,
	USAGE_STREAMING,
};

struct DXMaterial
{
	DXTexture texture;
	int startIndex;
	RenderPhase phase;
	vec4 color;
};

class DXModel
{
private:
	static const int MAX_MATERIALS = 8;

public:
	DXMaterial materials[MAX_MATERIALS];
	int numMaterials;
	mat4x4 modelMatrix;
	bool isLoaded, isBillboarded, inBackground;

	DXModel();
	void LoadAsMesh(const String& fileName, ModelUsage usage = USAGE_STATIC);
	void LoadAsQuad(const vec3& dimensions, const vec4& texCoord, ModelUsage usage = USAGE_DYNAMIC);
	void Unload();
	void Draw(RenderPhase pass, DXShader& shader) const;

private:
	ID3D11Buffer *verticesB, *indicesB;
	size_t vertexWidth, vertexSize;

	unsigned short numVertices, numIndices;

	void SetDefaults();
	void BufferData(const vec4* vertices, const vec2* texcoords, const unsigned short* elements, ModelUsage usage);
};

#endif
