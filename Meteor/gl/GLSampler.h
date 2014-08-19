#ifndef GL_SAMPLER_H
#define GL_SAMPLER_H

#include "GLInfo.h"

namespace GLSampler
{
	GLuint Create();
	void Destroy(GLuint sampler);
}

#endif