#include "DXPrimitives.h"

#include "DXInfo.h"
#include "DXUtils.h"

#include "../utilities/Logging.h"
#include "../utilities/Maths.h"

namespace DXPrimitives
{
	static const int BUFFER_MAX = 2048;
	ID3D11Buffer* buffer = NULL;

	int bufferFilled = 0;
	int vertexSize = 4 + 2;
	int vertexWidth = vertexSize * sizeof(float);

	void DrawPrimitives(const float vertexData[], int numVertices, D3D_PRIMITIVE_TOPOLOGY type);
}

void DXPrimitives::Initialize()
{
	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC vertexBufferDesc = {0};
	vertexBufferDesc.ByteWidth = sizeof(float) * vertexSize * BUFFER_MAX;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
 
	hr = _Device->CreateBuffer(&vertexBufferDesc, NULL, &buffer);
	if(FAILED(hr))
	{
		LOG_ISSUE("DIRECTX ERROR: %s - could not create Primitives vertex buffer",
			hresult_text(hr).Data());
		return;
	}
}

void DXPrimitives::Terminate()
{
	if(buffer != NULL)
		buffer->Release();
}

void DXPrimitives::DrawPrimitives(const float vertexData[], int numVertices, D3D_PRIMITIVE_TOPOLOGY type)
{
	if(bufferFilled + numVertices > BUFFER_MAX)
		bufferFilled = 0;

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if(SUCCEEDED(_DeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource)))
	{
		float* verticesPtr = ((float*) mappedResource.pData) + bufferFilled * vertexSize;
		memcpy(verticesPtr, vertexData, vertexWidth * numVertices);
		_DeviceContext->Unmap(buffer, 0);
	}

	UINT offset = 0;
	UINT stride = vertexWidth;
	_DeviceContext->IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	_DeviceContext->IASetPrimitiveTopology((D3D11_PRIMITIVE_TOPOLOGY) type);

	_DeviceContext->Draw(numVertices, bufferFilled);

	bufferFilled += numVertices;
}

void DXPrimitives::DrawTris(const vec4 vertices[], const vec2 texCoords[], int numVertices)
{
	float* verts = new float[numVertices * vertexSize];
	vec2 texCoord = vec2(0.5f, 0.5f);
	for(int i = 0; i < numVertices; i++)
	{
		verts[i*vertexSize+0] = vertices[i].x;
		verts[i*vertexSize+1] = vertices[i].y;
		verts[i*vertexSize+2] = vertices[i].z;
		verts[i*vertexSize+3] = vertices[i].w;

		if(texCoords != nullptr)
			texCoord = texCoords[i];
		verts[i*vertexSize+4] = texCoord.x;
		verts[i*vertexSize+5] = texCoord.y;
	}
	
	DrawPrimitives(verts, numVertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	delete[] verts;
}

void DXPrimitives::DrawQuads(const vec4 vertices[], const vec2 texCoords[], int numQuadVertices)
{
	//converts quads to tris while buffering
	int numTriVertices = numQuadVertices * 6 / 4;
	float* verts = new float[numTriVertices * vertexSize];
	const unsigned offsets[] = { 0, 1, 3, 3, 1, 2 };
	vec2 texCoord = vec2(0.5f, 0.5f);
	for(int i = 0; i < numTriVertices; i++)
	{
		verts[i*vertexSize+0] = vertices[4*(i/6)+offsets[i%6]].x;
		verts[i*vertexSize+1] = vertices[4*(i/6)+offsets[i%6]].y;
		verts[i*vertexSize+2] = vertices[4*(i/6)+offsets[i%6]].z;
		verts[i*vertexSize+3] = vertices[4*(i/6)+offsets[i%6]].w;

		if(texCoords != nullptr)
			texCoord = texCoords[4*(i/6)+offsets[i%6]];
		verts[i*vertexSize+4] = texCoord.x;
		verts[i*vertexSize+5] = texCoord.y;
	}
	DrawPrimitives(verts, numTriVertices, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	delete[] verts;
}

void DXPrimitives::DrawLines(const vec4 vertices[], int numVertices)
{
	float* verts = new float[numVertices * vertexSize];
	for(int i = 0; i < numVertices; i++)
	{
		verts[i*vertexSize+0] = vertices[i].x;
		verts[i*vertexSize+1] = vertices[i].y;
		verts[i*vertexSize+2] = vertices[i].z;
		verts[i*vertexSize+3] = vertices[i].w;
		verts[i*vertexSize+4] = 0.5f;
		verts[i*vertexSize+5] = 0.5f;
	}
	DrawPrimitives(verts, numVertices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	delete[] verts;
}

void DXPrimitives::DrawLineCircle(vec3 position, float radius, vec3 axisOfRotation)
{
	vec3 referenceAxis = UNIT_X;
	if(axisOfRotation == referenceAxis)
		referenceAxis = UNIT_Z;

	vec3 rotPoint = cross(axisOfRotation, referenceAxis) * radius;
	const int numSegments = 30;
	float* verts = new float[numSegments * vertexSize];
	for(int i = 0; i < numSegments; i++)
	{
		float theta = 2.0f * M_PI * float(i) / float(numSegments - 1);
		quaternion quat = quat_from_axis_angle(axisOfRotation, theta);
		vec3 point = quat * rotPoint;

		verts[i*vertexSize+0] = point.x + position.x;
		verts[i*vertexSize+1] = point.y + position.y;
		verts[i*vertexSize+2] = point.z + position.z;
		verts[i*vertexSize+3] = 1.0f;
		verts[i*vertexSize+4] = 0.5f;
		verts[i*vertexSize+5] = 0.5f;
	}
	DrawPrimitives(verts, numSegments, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	delete[] verts;
}

void DXPrimitives::DrawLineSphere(vec3 position, float radius)
{
	DrawLineCircle(position, radius, UNIT_X);
	DrawLineCircle(position, radius, UNIT_Y);
	DrawLineCircle(position, radius, UNIT_Z);
}

void DXPrimitives::DrawLineBox(vec3 center, vec3 dimensions, quaternion orientation)
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
	DrawLines(vertices, 24);
}
