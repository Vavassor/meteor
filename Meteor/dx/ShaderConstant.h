#ifndef SHADER_CONSTANT_H
#define SHADER_CONSTANT_H

#include "utilities/String.h"

enum ShaderType
{
	SHADER_VERT,
	SHADER_PIXEL,
};

class ShaderConstant
{
public:
	enum Type
	{
		FLOAT,
		VEC2,
		VEC3,
		VEC4,
		MAT4X4,
	};

	String name;
	float constant[16];
	int location;
	Type type;

	ShaderConstant();
	int GetSize() const;
};

#endif
