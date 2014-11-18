#ifndef GL_EMITTER_H
#define GL_EMITTER_H

#include "GLTexture.h"
#include "GLShader.h"

#include "../utilities/GLMath.h"

#include "../RenderPhase.h"

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

class GLEmitter
{
public:
	int numParticles, maxParticles;
	Particle* particles;

	mat4x4 modelMatrix;
	GLTexture texture;
	RenderPhase materialPhase;

	GLEmitter();
	void Load();
	void Unload();
	void SpawnParticle(int index);
	void DespawnParticle(int index);
	void Update();
	void Draw(RenderPhase phase, GLShader& shader) const;

protected:
	int vertexSize, vertexWidth;
	unsigned buffers[2];
	float* dataBuffer;

	void BufferData() const;
};

#endif
