#include "ShaderConstant.h"

ShaderConstant::ShaderConstant():
	location(-1),
	type(FLOAT)
{
	for(int i = 0; i < 16; i++)
		constant[i] = 0;
}

int ShaderConstant::GetSize() const
{
	switch(type)
	{
		case FLOAT:		return 1;
		case VEC2:		return 2;
		case VEC3:		return 3;
		case VEC4:		return 4;
		case MAT4X4:	return 16;
		default:		return 0;
	}
}
