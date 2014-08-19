#include "Terrain.h"
#include "GLMath.h"
#include "Noise.h"

namespace Terrain
{
	Chunk chunk;
}

void Terrain::Initialize()
{
	chunk.Create();
}

void Terrain::Terminate()
{
	chunk.Destroy();
}

void Terrain::Render()
{
	chunk.Render();
}

Terrain::Chunk::Chunk():
	vertexArray(0),
	numIndices(0)
{
	buffers[0] = buffers[1] = 0;
}

void Terrain::Chunk::Create()
{
	// Create the VAO
    glGenVertexArrays(1, &vertexArray); 
    glBindVertexArray(vertexArray);

	// Create the buffers for the vertex attributes
	glGenBuffers(2, buffers);

	static const int vertexSize = 4 + 2;
	static const int vertexWidth = vertexSize * sizeof(float);

	static const int GRID_X = 64;
	static const int GRID_Z = 64;

	int numVertices = 4 * GRID_X * GRID_Z;
	float* vertexData = new float[vertexSize * numVertices];

	numIndices = 6 * GRID_X * GRID_Z;
	unsigned short* indices = new unsigned short[numIndices];

	{
		vec3 corner = vec3(0.0f, 10.0f, 0.0f);
		float cellSize = 2.0f;
		float frequency = 0.1f;
		float amplitude = 4.0f;

		float* heightMap = new float[(GRID_X + 1) * (GRID_Z + 1)];
		for(int z = 0; z < GRID_Z + 1; z++)
		{
			for(int x = 0; x < GRID_X + 1; x++)
			{
				heightMap[z * (GRID_X + 1) + x] = amplitude * simplex_noise_2d(frequency * x, frequency * z);
			}
		}

		for(int z = 0; z < GRID_Z; z++)
		{
			for(int x = 0; x < GRID_X; x++)
			{
				int i = (z * GRID_X + x) * vertexSize * 4;
				vertexData[i+0] = corner.x + x * cellSize;
				vertexData[i+1] = corner.y + heightMap[z * (GRID_X + 1) + x];
				vertexData[i+2] = corner.z + z * cellSize;
				vertexData[i+3] = 1.0f;
				vertexData[i+4] = 0.0f;
				vertexData[i+5] = 0.0f;

				vertexData[i+6] = corner.x + x * cellSize + cellSize;
				vertexData[i+7] = corner.y + heightMap[z * (GRID_X + 1) + x + 1];
				vertexData[i+8] = corner.z + z * cellSize;
				vertexData[i+9] = 1.0f;
				vertexData[i+10] = 1.0f;
				vertexData[i+11] = 0.0f;

				vertexData[i+12] = corner.x + x * cellSize + cellSize;
				vertexData[i+13] = corner.y + heightMap[(z + 1) * (GRID_X + 1) + x + 1];
				vertexData[i+14] = corner.z + z * cellSize + cellSize;
				vertexData[i+15] = 1.0f;
				vertexData[i+16] = 1.0f;
				vertexData[i+17] = 1.0f;

				vertexData[i+18] = corner.x + x * cellSize;
				vertexData[i+19] = corner.y + heightMap[(z + 1) * (GRID_X + 1) + x];
				vertexData[i+20] = corner.z + z * cellSize + cellSize;
				vertexData[i+21] = 1.0f;
				vertexData[i+22] = 0.0f;
				vertexData[i+23] = 1.0f;
			}
		}

		delete[] heightMap;

		for(int i = 0; i < GRID_X * GRID_Z; i++)
		{
			 indices[(i*6)+0] = (i * 4) + 0;
			 indices[(i*6)+1] = (i * 4) + 3;
			 indices[(i*6)+2] = (i * 4) + 1;
			 indices[(i*6)+3] = (i * 4) + 1;
			 indices[(i*6)+4] = (i * 4) + 3;
			 indices[(i*6)+5] = (i * 4) + 2;
		}
	}

	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertexWidth * numVertices, vertexData, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertexWidth, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexWidth, (GLvoid*)(0 + 4 * sizeof(float)));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * numIndices, indices, GL_STATIC_DRAW);

	glBindVertexArray(0);

	delete[] vertexData;
	delete[] indices;
}

void Terrain::Chunk::Destroy()
{
	glDeleteBuffers(2, buffers);
	glDeleteVertexArrays(1, &vertexArray);
}

void Terrain::Chunk::Render() const
{
	glBindVertexArray(vertexArray);
	glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, nullptr);
}
