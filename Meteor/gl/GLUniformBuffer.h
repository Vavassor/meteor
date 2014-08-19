#ifndef GL_UNIFORM_BUFFER_H
#define GL_UNIFORM_BUFFER_H

#include <stddef.h>

#include "GLInfo.h"

namespace GLUniformBuffer
{
	GLuint Create(size_t capacity);
	void Destroy(GLuint buffer);
	void BufferData(GLuint buffer, void* data, size_t size);
}

#endif
