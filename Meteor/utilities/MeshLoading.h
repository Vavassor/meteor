#ifndef MESH_LOADING_H
#define MESH_LOADING_H

#include <vector>

#include "BString.h"
#include "GLMath.h"

struct MaterialInfo
{
	String texName;
	int startIndex;
	float alpha;
};

void index_VBO(
	const std::vector<vec4>& in_vertices,
	const std::vector<vec2>& in_uvs,
	const std::vector<vec3>& in_normals,
	std::vector<unsigned short> & out_indices, 
	std::vector<vec4>& out_vertices, 
	std::vector<vec2>& out_uvs, 
	std::vector<vec3>& out_normals);

int load_obj(const char* filename,
	std::vector<vec4>& vertices, 
	std::vector<vec3>& normals, 
	std::vector<vec2>& texcoords, 
	std::vector<unsigned short>& elements, 
	MaterialInfo* materials);

#endif
