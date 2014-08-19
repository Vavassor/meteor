#ifndef DX_EMITTER_H
#define DX_EMITTER_H

#include "../RenderPhase.h"

#include "GLMath.h"
#include "DXInfo.h"
#include "DXShader.h"
#include "DXTexture.h"

struct Particle
{
	float life;
	vec3 position;
	float scale;
	vec4 color;
	vec3 velocity;
	float rotation;
	float cameraDepth;
};

class DXEmitter
{
public:
	int numParticles, maxParticles;
	Particle* particles;
	mat4x4 modelMatrix;
	DXTexture texture;
	RenderPhase materialPhase;
	
	DXEmitter();
	void Load();
	void Unload();
	void Draw(RenderPhase phase, DXShader& shader) const;

protected:
	int vertexSize, vertexWidth;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;
	float* dataBuffer;
};

#endif
