#include "GLMesh.h"

GLMesh::GLMesh():
	sortingKey(0),
	vertexArray(0),
	startIndex(0),
	numIndices(0)
{
	materialBlock.color = VEC4_ONE;
	objectBlock.modelViewProjection = MAT_I;
}

void GLMesh::Draw() const
{
	glBindVertexArray(vertexArray);
	glDrawElementsBaseVertex(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, (GLvoid*)(startIndex * sizeof(GLushort)), 0);
}
