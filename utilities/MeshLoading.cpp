#include "MeshLoading.h"

#include "Logging.h"

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

#include <math.h>
#include <cstring>
#include <cstdlib>

using std::vector;
using std::map;
using std::string;

using std::ifstream;
using std::istringstream;

struct PackedVertex
{
	vec4 position;
	vec2 uv;
	vec3 normal;
	vec4 boneIndices;
	vec4 boneWeights;

	bool operator < (const PackedVertex& that) const
	{
		return memcmp((void*)this, (void*)&that, sizeof(PackedVertex)) > 0;
	}
};

static bool get_similar_vertex_index(
	PackedVertex& packed,
	map<PackedVertex,unsigned short>& VertexToOutIndex,
	unsigned short& result)
{
	map<PackedVertex,unsigned short>::iterator it = VertexToOutIndex.find(packed);
	if(it == VertexToOutIndex.end()) 
	{
		return false;
	}
	else 
	{
		result = it->second;
		return true;
	}
}

static void index_VBO(
	const vector<vec4> & in_vertices,
	const vector<vec2> & in_uvs,
	const vector<vec3> & in_normals,
	AutoArray<unsigned short> & out_indices,
	AutoArray<vec4> & out_vertices,
	AutoArray<vec2> & out_uvs,
	AutoArray<vec3> & out_normals
)
{
	map<PackedVertex,unsigned short> VertexToOutIndex;

	// For each input vertex
	unsigned int numVertices = in_vertices.size();
	for (unsigned int i = 0; i < numVertices; i++)
	{
		PackedVertex packed;
		packed.position = in_vertices[i];
		packed.uv = in_uvs[i];
		packed.normal = in_normals[i];
		
		// Try to find a similar vertex in out_XXXX
		unsigned short index;
		bool found = get_similar_vertex_index(packed, VertexToOutIndex, index);

		if(found)
		{
			// A similar vertex is already in the VBO, use it instead !
			out_indices.Push(index);
		}
		else
		{
			out_vertices.Push(in_vertices[i]);
			out_uvs.Push(in_uvs[i]);
			out_normals.Push(in_normals[i]);
			unsigned short newindex = (unsigned short) out_vertices.Count() - 1;
			out_indices.Push(newindex);
			VertexToOutIndex[packed] = newindex;
		}
	}
}

int load_obj(
	const char* filename,
	AutoArray<vec4> &vertices,
	AutoArray<vec3> &normals,
	AutoArray<vec2> &texcoords,
	AutoArray<unsigned short> &elements,
	MaterialInfo* materials)
{
	ifstream in(filename, std::ios::in);
	if(!in)
	{
		Log::Add(Log::ISSUE, "Cannot open %s", filename);
		return 0;
	}

	vector<vec4> tempVerts;
	vector<vec2> tempCoords;
	vector<vec3> tempNormals;
	vector<int> vertIndices, uvIndices, normIndices;
	string matlib;
	vector<string> mats;
	vector<int> matIndices;

	string line;
	while(getline(in, line)) 
	{
		if (line.substr(0,2) == "v ") 
		{
			istringstream s(line.substr(2));
			vec4 v; 
			s >> v.x;
			s >> v.y; 
			s >> v.z; 
			v.w = 1.0f;
			tempVerts.push_back(v);
		}  
		else if (line.substr(0,2) == "vt") 
		{
			istringstream s(line.substr(2));
			vec2 v;
			s >> v.x;
			s >> v.y;

			// ensure texcoords are between 0 and 1
			double intPart;
			v.x = modf(v.x, &intPart);
			v.y = modf(v.y, &intPart);
			if(v.x < 0.0f)
				v.x += 1.0f;
			if(v.y < 0.0f)
				v.y += 1.0f;

			tempCoords.push_back(v);
		}  
		else if (line.substr(0,2) == "vn") 
		{
			istringstream s(line.substr(2));
			vec3 v; 
			s >> v.x; 
			s >> v.y; 
			s >> v.z; 
			tempNormals.push_back(v);
		}  
		else if (line.substr(0,2) == "f ") 
		{
			istringstream s(line.substr(2));
			unsigned short a,b,c,i,j,k,x,y,z;

			s >> a; 
			s.ignore(1, L'/');
			s >> i;
			s.ignore(1, L'/');
			s >> x;
			s.ignore(16, L' ');

			s >> b; 
			s.ignore(1, L'/');
			s >> j; 
			s.ignore(1, L'/');
			s >> y;
			s.ignore(16, L' ');

			s >> c;
			s.ignore(1, L'/');
			s >> k;
			s.ignore(1, L'/');
			s >> z;
			s.ignore(16, L' ');

			vertIndices.push_back(a); 
			vertIndices.push_back(b);
			vertIndices.push_back(c);
			uvIndices.push_back(i);
			uvIndices.push_back(j);
			uvIndices.push_back(k);
			normIndices.push_back(x);
			normIndices.push_back(y);
			normIndices.push_back(z);
		}
		else if (line.substr(0,6) == "mtllib")
		{
			istringstream s(line.substr(6));
			s >> matlib;
		}
		else if (line.substr(0,6) == "usemtl")
		{
			istringstream s(line.substr(6));
			string material;
			s >> material;
			mats.push_back(material);
			matIndices.push_back(vertIndices.size());
		}
	}

	vector<vec4> out_vertices;
	vector<vec2> out_uvs;
	vector<vec3> out_normals;

	unsigned int lengthIndices = vertIndices.size();
	for(unsigned int i = 0; i < lengthIndices; i++)
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = vertIndices[i];
		unsigned int uvIndex = uvIndices[i];
		unsigned int normalIndex = normIndices[i];
		
		// Get the attributes thanks to the index
		vec4 vertex = tempVerts[vertexIndex-1];
		vec2 uv = tempCoords[uvIndex-1];
		vec3 normal = tempNormals[normalIndex-1];
		
		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs.push_back(uv);
		out_normals.push_back(normal);
	}

	index_VBO(out_vertices, out_uvs, out_normals, elements, vertices, texcoords, normals);

	int numMaterials = mats.size();
	for(int i = 0; i < numMaterials; i++)
	{
		materials[i].startIndex = matIndices[i];
	}

	string filePath = string(filename);
	string directory = filePath.substr(0, filePath.find_last_of(L'/') + 1);
	string mtlPath = directory + matlib;

	ifstream matIn(mtlPath, std::ios::in);
	if(!matIn)
	{
		Log::Add(Log::ISSUE, "material file: %s for model %s failed to load!",
			mtlPath.c_str(), filename);
	}

	int matInd = 0;
	while (getline(matIn, line))
	{
		if (line.substr(0,6) == "newmtl")
		{
			line = line.substr(7);
			for(size_t i = 0; i < mats.size(); i++)
			{
				if(mats[i] == line)
					matInd = i;
			}
		}
		else if(line.substr(0,6) == "map_Kd")
		{
			size_t slashInd = line.rfind("\\\\");
			line = line.substr(slashInd+2);
			materials[matInd].texName = line.c_str();
		}
		else if(line.substr(0,1) == "d")
		{
			line = line.substr(2);
			float alpha = (float) atof(line.c_str());
			materials[matInd].alpha = alpha;
		}
	}
	return numMaterials;
}
