#include "DXModel.h"

#include "DXUtils.h"

#include "utilities/MeshLoading.h"
#include "utilities/Logging.h"

#include "GlobalInfo.h"

DXModel::DXModel()
{
	SetDefaults();
}

void DXModel::SetDefaults()
{
	for(int i = 0; i < MAX_MATERIALS; i++)
	{
		materials[i].phase = PHASE_SOLID;
		materials[i].startIndex = 0;
		materials[i].color = VEC4_ONE;
	}
	numMaterials = 0;
	modelMatrix = MAT_I;

	isLoaded = false;
	isBillboarded = false;
	inBackground = false;

	verticesB = nullptr;
	indicesB = nullptr;
	vertexWidth = 0;
	vertexSize = 0;

	numVertices = 0;
	numIndices = 0;
}

void DXModel::LoadAsMesh(const String& fileName, ModelUsage usage)
{
	AutoArray<vec4> vertices;
	AutoArray<vec3> normals;
	AutoArray<vec2> texcoords;
	AutoArray<unsigned short> elements;

	String filePath = module_directory + fileName;
	MaterialInfo matInfo[MAX_MATERIALS];
	numMaterials = load_obj(filePath.Data(), vertices, normals, texcoords, elements, matInfo);

	for(int i = 0; i < numMaterials; i++)
	{
		materials[i].startIndex = matInfo[i].startIndex;
		materials[i].phase = (matInfo[i].alpha < 1.0f) ? PHASE_TRANSPARENT : PHASE_SOLID;
		materials[i].texture.Load("tex\\" + matInfo[i].texName);
		materials[i].color = VEC4_ONE;
	}

	numVertices = vertices.Count();
	numIndices = elements.Count();

	BufferData(vertices.First(), texcoords.First(), elements.First(), usage);

	isLoaded = true;
}

void DXModel::LoadAsQuad(const vec3& dimensions, const vec4& texCoord, ModelUsage usage)
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

	BufferData(vertices, texcoords, elements, usage);

	numMaterials = 1;
	materials[0].startIndex = 0;
	materials[0].phase = PHASE_MASKED;
	materials[0].color = VEC4_ONE;

	isLoaded = true;
}

void DXModel::BufferData(const vec4* vertices, const vec2* texcoords, const unsigned short* elements, ModelUsage usage)
{
	HRESULT hr = S_OK;

	vertexSize = 4 + 2;
	vertexWidth = sizeof(float) * vertexSize;
	float* vertexDataBuffer = new float[numVertices * vertexSize];
	for(int i = 0; i < numVertices; i++)
	{
		vertexDataBuffer[i*vertexSize+0] = vertices[i].x;
		vertexDataBuffer[i*vertexSize+1] = vertices[i].y;
		vertexDataBuffer[i*vertexSize+2] = vertices[i].z;
		vertexDataBuffer[i*vertexSize+3] = vertices[i].w;
		vertexDataBuffer[i*vertexSize+4] = texcoords[i].x;
		vertexDataBuffer[i*vertexSize+5] = texcoords[i].y;
	}

	D3D11_USAGE bufferUsage;
	switch(usage)
	{
		case USAGE_STATIC:
			bufferUsage = D3D11_USAGE_IMMUTABLE; break;
		case USAGE_DYNAMIC:
			bufferUsage = D3D11_USAGE_DEFAULT; break;
		case USAGE_STREAMING:
			bufferUsage = D3D11_USAGE_DYNAMIC; break;
	}

	D3D11_BUFFER_DESC vertexBufferDesc = {0};
	vertexBufferDesc.ByteWidth = vertexWidth * numVertices;
	vertexBufferDesc.Usage = bufferUsage;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = (usage == USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;
 
	D3D11_SUBRESOURCE_DATA vertexBufferData;
	vertexBufferData.pSysMem = vertexDataBuffer;
	vertexBufferData.SysMemPitch = 0;
	vertexBufferData.SysMemSlicePitch = 0;
 
	ID3D11Buffer* vertexBuffer;
	hr = _Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);
	delete[] vertexDataBuffer;
	if(FAILED(hr))
	{
		Log::Add(Log::ISSUE, "%s%s%s", "DIRECTX ERROR: %s - could not buffer vertex data", dxerr_text(hr));
		return;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * numIndices;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA indexBufferData;
	indexBufferData.pSysMem = elements;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
 
	ID3D11Buffer* indexBuffer;
	hr = _Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer);
	if(FAILED(hr))
	{
		Log::Add(Log::ISSUE, "DIRECTX ERROR: %s - could not buffer index data", dxerr_text(hr));
		return;
	}

	verticesB = vertexBuffer;
	indicesB = indexBuffer;
}

void DXModel::Unload()
{
	if(verticesB != nullptr)
		verticesB->Release();
	if(indicesB != nullptr)
		indicesB->Release();

	for(int i = 0; i < numMaterials; i++)
		materials[i].texture.Unload();

	SetDefaults();

	isLoaded = false;
}

void DXModel::Draw(RenderPhase phase, DXShader& shader) const
{
	shader.Bind();
	shader.SetConstantMatrix("model", SHADER_VERT, modelMatrix);

    UINT stride = vertexWidth;
    UINT offset = 0;
	_DeviceContext->IASetVertexBuffers(0, 1, &verticesB, &stride, &offset);
	_DeviceContext->IASetIndexBuffer(indicesB, DXGI_FORMAT_R16_UINT, 0);

    _DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	int numMats = numMaterials;
	for(int i = 0; i < numMats; i++)
	{
		if(phase != materials[i].phase)
			continue;

		shader.SetConstantVec4("color", SHADER_PIXEL, materials[i].color);
		shader.UpdateConstants();
		shader.SetTexture(0, materials[i].texture);

		unsigned start, end;
		if(i == numMaterials - 1)
		{
			start = materials[i].startIndex;
			end = numIndices;
		}
		else 
		{
			start = materials[i].startIndex;
			end = materials[i + 1].startIndex;
		}
		unsigned range = end - start;
		_DeviceContext->DrawIndexed(range, start, 0);
	}
}
