#include "GLModel.h"

#include "../utilities/String.h"
#include "../utilities/MeshLoading.h"

GLModel::GLModel()
{
	SetDefaults();
}

void GLModel::SetDefaults()
{
	numMaterials = 0;
	modelMatrix = MAT_I;

	isLoaded = false;
	isBillboarded = false;
	inBackground = false;

	vertexArray = 0;
	verticesV = 0;
	elementsV = 0;

	numIndices = 0;
	numVertices = 0;
}

void GLModel::LoadAsMesh(const String& filename)
{
	String path("meshes/");
	path.Append(filename);

	AutoArray<vec4> vertices;
	AutoArray<vec3> normals;
	AutoArray<vec2> texcoords;
	AutoArray<unsigned short> elements;

	MaterialInfo matInfo[MAX_MATERIALS];
	numMaterials = load_obj(path.Data(), vertices, normals, texcoords, elements, matInfo);

	for(int i = 0; i < numMaterials; i++)
	{
		materials[i].startIndex = matInfo[i].startIndex;
		materials[i].phase = (matInfo[i].alpha < 1.0f)? PHASE_TRANSPARENT : PHASE_SOLID;
		materials[i].color = VEC4_ONE;
		materials[i].texture.Load(matInfo[i].texName);
	}

	numVertices = vertices.Count();
	numIndices = elements.Count();

	BufferData(vertices.First(), texcoords.First(), elements.First());

	isLoaded = true;
}

void GLModel::LoadAsQuad(const vec3& dimensions, const vec4& texCoord, bool isStatic)
{
	numVertices = 4;
	numIndices = 6;

	const vec4 vertices[4] = { 
		vec4(-dimensions.x / 2.0f, 0.0f, 0.0f, 1.0f),
		vec4(dimensions.x / 2.0f, 0.0f, 0.0f, 1.0f),
		vec4(dimensions.x / 2.0f, dimensions.y, 0.0f, 1.0f),
		vec4(-dimensions.x / 2.0f, dimensions.y, 0.0f, 1.0f)
	};
	const vec2 texcoords[4] = { 
		vec2(texCoord.x, texCoord.y), 
		vec2(texCoord.x + texCoord.z, texCoord.y), 
		vec2(texCoord.x + texCoord.z, texCoord.y + texCoord.w),
		vec2(texCoord.x, texCoord.y + texCoord.w)
	};
	const unsigned short elements[6] = { 0, 3, 1, 1, 3, 2 };

	BufferData(vertices, texcoords, elements, isStatic);

	numMaterials = 1;
	materials[0].startIndex = 0;
	materials[0].phase = PHASE_MASKED;
	materials[0].color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

	isLoaded = true;
}

void GLModel::BufferData(const vec4* vertices, const vec2* texcoords, const unsigned short* elements, bool isStatic)
{
	// Create the VAO
    glGenVertexArrays(1, &vertexArray); 
    glBindVertexArray(vertexArray);

    // Create the buffers for the vertices attributes
	glGenBuffers(1, &verticesV);
	glGenBuffers(1, &elementsV);

	static const int vertexSize = 4 + 2;
	static const int vertexWidth = vertexSize * sizeof(float);

	float* verts = new float[numVertices * vertexSize];
	for(int i = 0; i < numVertices; i++)
	{
		verts[i*vertexSize+0] = vertices[i].x;
		verts[i*vertexSize+1] = vertices[i].y;
		verts[i*vertexSize+2] = vertices[i].z;
		verts[i*vertexSize+3] = vertices[i].w;
		verts[i*vertexSize+4] = texcoords[i].x;
		verts[i*vertexSize+5] = texcoords[i].y;
	}

	GLenum mode = (isStatic) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;

	glBindBuffer(GL_ARRAY_BUFFER, verticesV);
	glBufferData(GL_ARRAY_BUFFER, numVertices * vertexWidth, verts, mode);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertexWidth, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexWidth, (GLvoid*)(NULL + sizeof(float) * 4));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementsV);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * numIndices, elements, GL_STATIC_DRAW);

	glBindVertexArray(0);

	delete[] verts;
}

void GLModel::Unload()
{
	glDeleteBuffers(1, &verticesV);
	glDeleteBuffers(1, &elementsV);
	glDeleteVertexArrays(1, &vertexArray);

	for(int i = 0; i < numMaterials; i++)
		materials[i].texture.Unload();

	SetDefaults();

	isLoaded = false;
}
