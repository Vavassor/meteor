#include "GLEmitter.h"

#include "GLInfo.h"

#include "utilities/Maths.h"

#include <cstring>

GLEmitter::GLEmitter()
{
	numParticles = maxParticles = 0;
	particles = nullptr;
	dataBuffer = nullptr;
	modelMatrix = MAT_I;
	materialPhase = PHASE_TRANSPARENT;

	vertexSize = 4 + 4 + 2;
	vertexWidth = sizeof(float) * vertexSize;
	for(int i = 0; i < 2; i++)
		buffers[i] = 0;
}

void GLEmitter::Load()
{
	particles = new Particle[maxParticles];
	dataBuffer = new float[maxParticles * 4 * vertexSize];

	glGenBuffers(2, buffers);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, maxParticles * 4 * vertexWidth, nullptr, GL_STREAM_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// initialize index buffer
	unsigned short* indices = new unsigned short[maxParticles * 6];
	for (int i = 0; i < maxParticles; i++)
	{
		unsigned short n = i * 4;
		int index = i * 6;
		indices[index] = n;
		indices[index+1] = n + 1;
		indices[index+2] = n + 2;
		indices[index+3] = n + 1;
		indices[index+4] = n + 3; 
		indices[index+5] = n + 2;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * maxParticles * 6, nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	delete[] indices;
}

void GLEmitter::Unload()
{
	delete[] particles;
	delete[] dataBuffer;

	glDeleteBuffers(2, buffers);
}

void GLEmitter::SpawnParticle(int index)
{
	// Swap it with the first inactive particle
	// right after the active ones.
	Particle temp = particles[numParticles];
	particles[numParticles++] = particles[index];
	particles[index] = temp;
}

void GLEmitter::DespawnParticle(int index)
{
	// Swap it with the last active particle
	// right before the inactive ones.
	numParticles--;
	Particle temp = particles[numParticles];
	particles[numParticles] = particles[index];
	particles[index] = temp;
}

void GLEmitter::Update()
{
	for(int i = 0; i < numParticles; i++)
	{
		
	}
	BufferData();
}

void GLEmitter::BufferData() const
{
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);

	int length = numParticles * 4 * vertexWidth;
	GLvoid* mappedBuffer = glMapBufferRange(GL_ARRAY_BUFFER, 0, length,
		GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	if(mappedBuffer != nullptr)
	{
		memcpy(mappedBuffer, dataBuffer, length);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLEmitter::Draw(RenderPhase phase, GLShader& shader) const
{
	if(materialPhase != phase) return;

	BufferData();

	shader.Bind();
	shader.SetTexture(0, texture);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexAttribPointer(shader.attributeLocations[0], 4, GL_FLOAT, GL_FALSE, 4 * 10, 0);
	glVertexAttribPointer(shader.attributeLocations[1], 4, GL_FLOAT, GL_FALSE, 4 * 10, (GLvoid*)(4 * 4));
	glVertexAttribPointer(shader.attributeLocations[2], 2, GL_FLOAT, GL_FALSE, 4 * 10, (GLvoid*)(8 * 4));

	glEnableVertexAttribArray(shader.attributeLocations[0]);
	glEnableVertexAttribArray(shader.attributeLocations[1]);
	glEnableVertexAttribArray(shader.attributeLocations[2]);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);

	glDrawElements(GL_TRIANGLES, 6 * numParticles, GL_UNSIGNED_SHORT, 0);

	glDisableVertexAttribArray(shader.attributeLocations[0]);
	glDisableVertexAttribArray(shader.attributeLocations[1]);
	glDisableVertexAttribArray(shader.attributeLocations[2]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glUseProgram(0);
}
