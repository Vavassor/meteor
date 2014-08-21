#include "GLPrimitives.h"

#include "GLInfo.h"

#include "utilities/Maths.h"

#include <cstring>

namespace GLPrimitives
{
	static const int BUFFER_MAX = 2096;
	GLuint vertexBuffer = 0;
	GLuint vertexArray = 0;
	int bufferFilled = 0;

	int vertexSize = 4 + 2;
	int vertexWidth = vertexSize * sizeof(float);

	float* vertexData;
	int numData = 0;
	GLenum primitiveType = 0;
}

void GLPrimitives::Initialize()
{
	glGenVertexArrays(1, &vertexArray); 
    glBindVertexArray(vertexArray);

	glGenBuffers(1, &vertexBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, BUFFER_MAX * vertexWidth, nullptr, GL_DYNAMIC_DRAW);
	
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, vertexWidth, 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, vertexWidth, (GLvoid*)(sizeof(float) * 4));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	vertexData = new float[BUFFER_MAX * vertexSize];
}

void GLPrimitives::Terminate()
{
	glDeleteBuffers(1, &vertexBuffer);
	glDeleteVertexArrays(1, &vertexArray);

	delete[] vertexData;
}

void GLPrimitives::Draw()
{
	if(bufferFilled + numData > BUFFER_MAX)
		bufferFilled = 0;

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

	int length = numData * vertexWidth;
	GLvoid* mappedBuffer = glMapBufferRange(GL_ARRAY_BUFFER, bufferFilled * vertexWidth, length, 
		GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	if(mappedBuffer != nullptr)
	{
		memcpy(mappedBuffer, vertexData, length);
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	glBindVertexArray(vertexArray);
	glDrawArrays(primitiveType, bufferFilled, numData);

	bufferFilled += numData;
	numData = 0;
}

void GLPrimitives::AddTris(const vec4 vertices[], const vec2 texCoords[], int numVertices)
{
	const int dataOffset = numData * vertexSize;
	for(int i = 0; i < numVertices; i++)
	{
		vertexData[dataOffset+i*vertexSize+0] = vertices[i].x;
		vertexData[dataOffset+i*vertexSize+1] = vertices[i].y;
		vertexData[dataOffset+i*vertexSize+2] = vertices[i].z;
		vertexData[dataOffset+i*vertexSize+3] = vertices[i].w;

		if(texCoords != nullptr)
		{
			vertexData[dataOffset+i*vertexSize+4] = texCoords[i].x;
			vertexData[dataOffset+i*vertexSize+5] = texCoords[i].y;
		}
		else
		{
			vertexData[dataOffset+i*vertexSize+4] = 0.5f;
			vertexData[dataOffset+i*vertexSize+5] = 0.5f;
		}
	}
	numData += numVertices;
	primitiveType = GL_TRIANGLES;
}

void GLPrimitives::AddQuads(const vec4 vertices[], const vec2 texCoords[], int numQuadVertices)
{
	//converts quads to tris while buffering
	int numTriVertices = numQuadVertices * 6 / 4;

	const unsigned offsets[] = { 0, 1, 3, 3, 1, 2 };
	const int dataOffset = numData * vertexSize;
	for(int i = 0; i < numTriVertices; i++)
	{
		vertexData[dataOffset+i*vertexSize+0] = vertices[4*(i/6)+offsets[i%6]].x;
		vertexData[dataOffset+i*vertexSize+1] = vertices[4*(i/6)+offsets[i%6]].y;
		vertexData[dataOffset+i*vertexSize+2] = vertices[4*(i/6)+offsets[i%6]].z;
		vertexData[dataOffset+i*vertexSize+3] = vertices[4*(i/6)+offsets[i%6]].w;

		if(texCoords != nullptr)
		{
			vertexData[dataOffset+i*vertexSize+4] = texCoords[4*(i/6)+offsets[i%6]].x;
			vertexData[dataOffset+i*vertexSize+5] = texCoords[4*(i/6)+offsets[i%6]].y;
		}
		else
		{
			vertexData[dataOffset+i*vertexSize+4] = 0.5f;
			vertexData[dataOffset+i*vertexSize+5] = 0.5f;
		}
	}
	numData += numTriVertices;
	primitiveType = GL_TRIANGLES;
}

void GLPrimitives::AddLines(const vec4 vertices[], int numVertices)
{
	const int dataOffset = numData * vertexSize;
	for(int i = 0; i < numVertices; i++)
	{
		vertexData[dataOffset+i*vertexSize+0] = vertices[i].x;
		vertexData[dataOffset+i*vertexSize+1] = vertices[i].y;
		vertexData[dataOffset+i*vertexSize+2] = vertices[i].z;
		vertexData[dataOffset+i*vertexSize+3] = vertices[i].w;
		vertexData[dataOffset+i*vertexSize+4] = 0.5f;
		vertexData[dataOffset+i*vertexSize+5] = 0.5f;
	}
	numData += numVertices;
	primitiveType = GL_LINES;
}

void GLPrimitives::AddLineCircle(vec3 position, float radius, vec3 axisOfRotation)
{
	vec3 referenceAxis = UNIT_X;
	if(axisOfRotation == referenceAxis)
		referenceAxis = UNIT_Z;

	vec3 rotPoint = cross(axisOfRotation, referenceAxis) * radius;
	vec3 lastPoint = rotPoint;
	const int dataOffset = numData * vertexSize;
	const int numSegments = 30;
	for(int i = 0; i < 2 * numSegments; i++)
	{
		float theta = M_TAU * float(i / 2) / float(numSegments - 1);
		quaternion quat = quat_from_axis_angle(axisOfRotation, theta);
		vec3 point = quat * rotPoint;

		vertexData[dataOffset+i*vertexSize+0] = point.x + position.x;
		vertexData[dataOffset+i*vertexSize+1] = point.y + position.y;
		vertexData[dataOffset+i*vertexSize+2] = point.z + position.z;
		vertexData[dataOffset+i*vertexSize+3] = 1.0f;
		vertexData[dataOffset+i*vertexSize+4] = 0.5f;
		vertexData[dataOffset+i*vertexSize+5] = 0.5f;

		i++;

		vertexData[dataOffset+i*vertexSize+0] = lastPoint.x + position.x;
		vertexData[dataOffset+i*vertexSize+1] = lastPoint.y + position.y;
		vertexData[dataOffset+i*vertexSize+2] = lastPoint.z + position.z;
		vertexData[dataOffset+i*vertexSize+3] = 1.0f;
		vertexData[dataOffset+i*vertexSize+4] = 0.5f;
		vertexData[dataOffset+i*vertexSize+5] = 0.5f;

		lastPoint = point;
	}
	numData += 2 * numSegments;
	primitiveType = GL_LINES;
}

void GLPrimitives::AddLineSphere(vec3 position, float radius)
{
	AddLineCircle(position, radius, UNIT_X);
	AddLineCircle(position, radius, UNIT_Y);
	AddLineCircle(position, radius, UNIT_Z);
}

void GLPrimitives::AddLineBox(vec3 center, vec3 dimensions, quaternion orientation)
{
	vec3 min = -(dimensions / 2.0f);
	vec3 max = (dimensions / 2.0f);

	vec3 corners[8];
	corners[0] = max;
	corners[1] = max - vec3(0.0f, 0.0f, dimensions.z);
	corners[2] = max - vec3(dimensions.x, 0.0f, dimensions.z);
	corners[3] = max - vec3(dimensions.x, 0.0f, 0.0f);
	corners[4] = min + vec3(dimensions.x, 0.0f, dimensions.z);
	corners[5] = min + vec3(dimensions.x, 0.0f, 0.0f);
	corners[6] = min;
	corners[7] = min + vec3(0.0f, 0.0f, dimensions.z);

	for(int i = 0; i < 8; i++)
		corners[i] = center + (orientation * corners[i]);

	vec4 vertices[24];
	{
		// top rectangle
		for(int i = 0; i < 3; i++)
		{
			vertices[i*2+0] = vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
			vertices[i*2+1] = vec4(corners[i + 1].x, corners[i + 1].y, corners[i + 1].z, 1.0f);
		}
		vertices[6] = vec4(corners[0].x, corners[0].y, corners[0].z, 1.0f);
		vertices[7] = vec4(corners[3].x, corners[3].y, corners[3].z, 1.0f);

		// bottom rectangle
		for(int i = 4; i < 7; i++)
		{
			vertices[i*2+0] = vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
			vertices[i*2+1] = vec4(corners[i + 1].x, corners[i + 1].y, corners[i + 1].z, 1.0f);
		}
		vertices[14] = vec4(corners[4].x, corners[4].y, corners[4].z, 1.0f);
		vertices[15] = vec4(corners[7].x, corners[7].y, corners[7].z, 1.0f);

		// sides
		for(int i = 0; i < 4; i++)
		{
			vertices[16+i*2+0] = vec4(corners[i].x, corners[i].y, corners[i].z, 1.0f);
			vertices[16+i*2+1] = vec4(corners[i + 4].x, corners[i + 4].y, corners[i + 4].z, 1.0f);
		}
	}
	AddLines(vertices, 24);
}
