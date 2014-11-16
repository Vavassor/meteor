#include "GLShader.h"

#include "utilities/Logging.h"

#include <cstdio>

GLShader::GLShader():
	vertexShader(0),
	fragmentShader(0),
	program(0),
	numAttributes(0)
{
	for(int i = 0; i < MAX_ATTRIBUTES; i++)
		attributeLocations[i] = -1;
}

GLShader::operator GLuint() const
{
	return program;
}

bool GLShader::Load(const String& vertexFileName, const String& pixelFileName)
{
	// unload any existing program data
	if(vertexShader || fragmentShader || program)
	{
		Unload();
	}

	// load individual shaders
	bool error = false;

	error |= ((vertexShader = LoadShaderGL(GL_VERTEX_SHADER, vertexFileName)) == 0);
	error |= ((fragmentShader = LoadShaderGL(GL_FRAGMENT_SHADER, pixelFileName)) == 0);

	if(error)
	{
		Unload();
		return false;
	}

	// create program using shaders
	program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);
	glLinkProgram(program);

	int Param = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &Param);

	if(Param == GL_FALSE)
	{
		int InfoLogLength = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char* InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetProgramInfoLog(program, InfoLogLength, &CharsWritten, InfoLog);
			Log::Add(Log::ISSUE, "Error linking program (%s, %s)\n%s",
				vertexFileName.Data(), pixelFileName.Data(), InfoLog);
			delete[] InfoLog;
		}

		Unload();

		return false;
	}
	return true;
}

GLuint GLShader::LoadShaderGL(GLenum Type, const String& shaderFileName)
{
	String path("shaders/gl/");
	path.Append(shaderFileName);

	FILE* File = fopen(path.Data(), "rb");
	if(File == NULL)
	{
		LOG_ISSUE("couldn't load shader file: %s", shaderFileName.Data());
		return 0;
	}

	fseek(File, 0, SEEK_END);
	long Size = ftell(File);
	fseek(File, 0, SEEK_SET);
	char *Source = new char[Size + 1];
	fread(Source, 1, Size, File);
	fclose(File);
	Source[Size] = 0;

	GLuint Shader;

	Shader = glCreateShader(Type);
	glShaderSource(Shader, 1, (const char**)&Source, NULL);
	delete [] Source;
	glCompileShader(Shader);

	int Param = 0;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &Param);

	if(Param == GL_FALSE)
	{
		int InfoLogLength = 0;
		glGetShaderiv(Shader, GL_INFO_LOG_LENGTH, &InfoLogLength);
	
		if(InfoLogLength > 0)
		{
			char* InfoLog = new char[InfoLogLength];
			int CharsWritten  = 0;
			glGetShaderInfoLog(Shader, InfoLogLength, &CharsWritten, InfoLog);
			LOG_ISSUE("Error compiling shader %s\n%s", shaderFileName.Data(), InfoLog);
			delete[] InfoLog;
		}

		glDeleteShader(Shader);

		return 0;
	}
	return Shader;
}

void GLShader::Unload()
{
	// delete all existing shader data
	if(program != 0)
	{
		if(vertexShader != 0)
			glDetachShader(program, vertexShader);
		if(fragmentShader != 0)
			glDetachShader(program, fragmentShader);

		glDeleteProgram(program);
	}

	if(vertexShader != 0)
		glDeleteShader(vertexShader);
	if(fragmentShader != 0)
		glDeleteShader(fragmentShader);

	// reset to defaults
	vertexShader = 0;
	fragmentShader = 0;
	program = 0;

	numAttributes = 0;
	for(int i = 0; i < MAX_ATTRIBUTES; i++)
		attributeLocations[i] = -1;
}

void GLShader::Bind() const
{
	glUseProgram(program);
}

void GLShader::AddAttribute(const char* name)
{
	if(numAttributes < MAX_ATTRIBUTES)
		attributeLocations[numAttributes++] = glGetAttribLocation(program, name);
}

void GLShader::SetTexture(unsigned int slot, const GLTexture& texture) const
{
	if(slot >= (unsigned int) gl_max_combined_texture_image_units) return;

	static unsigned int lastSlot = 0;
	if(slot != lastSlot)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		lastSlot = slot;
	}
	glBindTexture(GL_TEXTURE_2D, texture);
}

void GLShader::SetUniformInt(const char* name, int uniform) const
{
	GLint location = glGetUniformLocation(program, name);
	glUniform1i(location, uniform);
}

void GLShader::BindUniformBuffer(const char* blockName, GLuint buffer, GLuint bindingPoint) const
{
	GLuint blockIndex = glGetUniformBlockIndex(program, blockName);
	glUniformBlockBinding(program, blockIndex, bindingPoint);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, buffer);
}
