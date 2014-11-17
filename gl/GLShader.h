#ifndef GL_SHADER_H
#define GL_SHADER_H

#include "GLInfo.h"
#include "GLTexture.h"

#include "../utilities/GLMath.h"

class GLShader
{
private:
	static const int MAX_ATTRIBUTES = 4;

public:
	GLint attributeLocations[MAX_ATTRIBUTES];

	GLShader();
	operator GLuint() const;
	bool Load(const String& vertexFileName, const String& pixelFileName);
	void Unload();
	void Bind() const;
	void AddAttribute(const char* name);
	void SetTexture(unsigned int slot, const GLTexture& texture) const;
	void SetUniformInt(const char* name, int uniform) const;
	void BindUniformBuffer(const char* blockName, GLuint buffer, GLuint bindingPoint) const;

private:
	GLuint vertexShader, fragmentShader, program;
	int numAttributes;

	GLuint LoadShaderGL(GLenum type, const String& shaderFileName);
};

#endif
