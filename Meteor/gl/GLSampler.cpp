#include "GLSampler.h"

GLuint GLSampler::Create()
{
	GLuint sampler;
	glGenSamplers(1, &sampler);
	return sampler;
}

void GLSampler::Destroy(GLuint sampler)
{
	glDeleteSamplers(1, &sampler);
}