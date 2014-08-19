#include "GLUniformBuffer.h"

GLuint GLUniformBuffer::Create(size_t capacity)
{
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, capacity, nullptr, GL_STREAM_DRAW);
	return buffer;
}

void GLUniformBuffer::Destroy(GLuint buffer)
{
	glDeleteBuffers(1, &buffer);
}

void GLUniformBuffer::BufferData(GLuint buffer, void* data, size_t size)
{
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
}
