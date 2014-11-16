#include "DXEmitter.h"

#include "DXUtils.h"

#include "utilities/Logging.h"

DXEmitter::DXEmitter()
{
	numParticles = maxParticles = 0;
	particles = nullptr;
	modelMatrix = MAT_I;
	materialPhase = PHASE_TRANSPARENT;

	vertexSize = 4 + 4 + 2;
	vertexWidth = sizeof(float) * vertexSize;
	vertexBuffer = indexBuffer = NULL;
	dataBuffer = nullptr;
}

void DXEmitter::Load()
{
	particles = new Particle[maxParticles];
	dataBuffer = new float[maxParticles * 4 * vertexSize];

	HRESULT hr = S_OK;

	D3D11_BUFFER_DESC vertexBufferDesc = {0};
	vertexBufferDesc.ByteWidth = maxParticles * 4 * vertexWidth;
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
 
	D3D11_SUBRESOURCE_DATA vertexBufferData = {0};
	vertexBufferData.pSysMem = NULL;

	hr = _Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &vertexBuffer);
	if(FAILED(hr))
	{
		Log::Add(Log::ISSUE, "DIRECTX ERROR: %s - could not buffer vertex data", dxerr_text(hr));
		return;
	}

	D3D11_BUFFER_DESC indexBufferDesc = {0};
	indexBufferDesc.ByteWidth = sizeof(unsigned short) * maxParticles * 6;
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	unsigned short* indices = new unsigned short[maxParticles * 6];
	for (int i = 0; i < maxParticles; i++)
	{
		unsigned short n = i * 4;
		int index = i * 6;
		indices[index] = n;
		indices[index+1] = n + 1;
		indices[index+2] = n + 2; 
		indices[index+3] = n + 1;
		indices[index+4] = n + 3; 
		indices[index+5] = n + 2;
	}

	D3D11_SUBRESOURCE_DATA indexBufferData = {0};
	indexBufferData.pSysMem = indices;
 
	hr = _Device->CreateBuffer(&indexBufferDesc, &indexBufferData, &indexBuffer);
	if(FAILED(hr))
	{
		Log::Add(Log::ISSUE, "DIRECTX ERROR: %s - could not buffer index data", dxerr_text(hr));
	}
	delete[] indices;
}

void DXEmitter::Unload()
{
	delete[] particles;
	delete[] dataBuffer;

	texture.Unload();

	if(vertexBuffer != NULL)
		vertexBuffer->Release();
	if(indexBuffer != NULL)
		indexBuffer->Release();
}

void DXEmitter::Draw(RenderPhase phase, DXShader& shader) const
{
	if(materialPhase != phase) return;

	D3D11_MAPPED_SUBRESOURCE subData;
	if(SUCCEEDED(_DeviceContext->Map(vertexBuffer, 0, D3D11_MAP_WRITE, 0, &subData)))
	{
		memcpy(subData.pData, dataBuffer, numParticles * 4 * vertexWidth);
		_DeviceContext->Unmap(vertexBuffer, 0);
	}

	shader.Bind();
	shader.SetConstantMatrix("model", SHADER_VERT, modelMatrix);
	shader.UpdateConstants();
	shader.SetTexture(0, texture);

	UINT stride = vertexWidth;
	UINT offset = 0;
	_DeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	_DeviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);

	_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_DeviceContext->DrawIndexed(6 * numParticles, 0, 0);
}
