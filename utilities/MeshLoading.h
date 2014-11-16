#ifndef MESH_LOADING_H
#define MESH_LOADING_H

#include "collections/AutoArray.h"

#include "String.h"
#include "GLMath.h"

struct MaterialInfo
{
	String texName;
	int startIndex;
	float alpha;
};

int load_obj(const char* filename,
	AutoArray<vec4>& vertices,
	AutoArray<vec3>& normals,
	AutoArray<vec2>& texcoords,
	AutoArray<unsigned short>& elements, 
	MaterialInfo* materials);

#endif
