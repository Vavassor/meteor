#include "DXShader.h"

#include "DXUtils.h"

#include "utilities/FileHandling.h"
#include "utilities/Logging.h"

float DXShader::vertBuffer[MAX_VERTEX_CONSTANTS * 16];
float DXShader::pixelBuffer[MAX_PIXEL_CONSTANTS * 16];

DXShader::DXShader()
{
	SetDefaults();
}

void DXShader::SetDefaults()
{
	vertexShader = nullptr;
	pixelShader = nullptr;
	inputLayout = nullptr;
	vertCB = nullptr;
	pixelCB = nullptr;
	layoutType = 0;

	numVertexConstants = 0;
	numPixelConstants = 0;
}

bool DXShader::Load(const String& vertexFileName, const String& pixelFileName)
{
	HRESULT hr = S_OK;

	String vertexFile("shaders/dx/");
	vertexFile.Append(vertexFileName);

	// create vertex shader
	void* vBuffer = nullptr;
	unsigned vSize = load_binary_file(&vBuffer, vertexFile.Data());
	hr = _Device->CreateVertexShader(vBuffer, vSize, NULL, &vertexShader);
	if(FAILED(hr))
	{
		delete[] vBuffer;
		LOG_ISSUE("DIRECTX ERROR: %s - failed to load vertex shader: %s",
			dxerr_text(hr), vertexFileName);
		return false;
	}

	// create input layout
	const D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
	const D3D11_INPUT_ELEMENT_DESC particleVertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,	D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",	  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,		 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

	const D3D11_INPUT_ELEMENT_DESC* inputLayoutDesc = (layoutType == 0) ? basicVertexLayoutDesc : particleVertexLayoutDesc;
	unsigned inputDescSize = (layoutType == 0) ? ARRAYSIZE(basicVertexLayoutDesc) : ARRAYSIZE(particleVertexLayoutDesc);

	hr = _Device->CreateInputLayout(inputLayoutDesc, inputDescSize, vBuffer, vSize, &inputLayout);
	delete[] vBuffer;
	if(FAILED(hr))
	{
		LOG_ISSUE("DIRECTX ERROR: %s - failed to create input layout "
			"for vertex shader: %s", dxerr_text(hr), vertexFileName);
		return false;
	}

	// create pixel shader
	String pixelFile("shaders/dx/");
	pixelFile.Append(pixelFileName);

	void* psBuffer = nullptr;
	unsigned psSize = load_binary_file(&psBuffer, pixelFile.Data());
    hr = _Device->CreatePixelShader(psBuffer, psSize, NULL, &pixelShader);
	delete[] psBuffer;
	if(FAILED(hr))
	{
		LOG_ISSUE("DIRECTX ERROR: %s - failed to load pixel shader: %s",
			dxerr_text(hr), pixelFileName);
		return false;
	}

	// initialize constants
	numVertexConstants = 3;
	vertexConstants[0].name = "model";
	vertexConstants[0].type = ShaderConstant::MAT4X4;
	vertexConstants[1].name = "view";
	vertexConstants[1].type = ShaderConstant::MAT4X4;
	vertexConstants[2].name = "projection";
	vertexConstants[2].type = ShaderConstant::MAT4X4;
	
	numPixelConstants = 1;
	pixelConstants[0].name = "color";
	pixelConstants[0].type = ShaderConstant::VEC4;

	int count = 0;
	for(int i = 0; i < numVertexConstants; i++)
	{
		vertexConstants[i].location = count;
		count += vertexConstants[i].GetSize();
	}
	count = 0;
	for(int i = 0; i < numPixelConstants; i++)
	{
		pixelConstants[i].location = count;
		count += pixelConstants[i].GetSize();
	}

	// create constant buffers
	int vertexConstantBufferSize = 0;
	for(int i = 0; i < numVertexConstants; i++)
		vertexConstantBufferSize += vertexConstants[i].GetSize();

	D3D11_BUFFER_DESC constantBufferDesc = {0};
    constantBufferDesc.ByteWidth = sizeof(float) * vertexConstantBufferSize;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    _Device->CreateBuffer(&constantBufferDesc, nullptr, &vertCB);
	if(FAILED(hr))
	{
		LOG_ISSUE("DIRECTX ERROR: %s - failed to create constant buffer "
			"for shaders: %s and %s", dxerr_text(hr), vertexFileName, pixelFileName);
		return false;
	}

	int pixelConstantBufferSize = 0;
	for(int i = 0; i < numPixelConstants; i++)
		pixelConstantBufferSize += pixelConstants[i].GetSize();

	D3D11_BUFFER_DESC pixelCBDesc = {0};
    pixelCBDesc.ByteWidth = sizeof(float) * pixelConstantBufferSize;
    pixelCBDesc.Usage = D3D11_USAGE_DYNAMIC;
    pixelCBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pixelCBDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	_Device->CreateBuffer(&pixelCBDesc, nullptr, &pixelCB);
	if(FAILED(hr))
	{
		LOG_ISSUE("DIRECTX ERROR: %s - failed to create pixel constant buffer "
			"for shaders: %s and %s", dxerr_text(hr), vertexFileName, pixelFileName);
		return false;
	}
	return true;
}

void DXShader::Unload()
{
	if(vertCB != nullptr)
		vertCB->Release();
	if(pixelCB != nullptr)
		pixelCB->Release();
	if(inputLayout != nullptr)
		inputLayout->Release();
	if(pixelShader != nullptr)
		pixelShader->Release();
	if(vertexShader != nullptr)
		vertexShader->Release();

	SetDefaults();
}

void DXShader::Bind() const
{
	// must unset all ShaderResourceViews before calling PSSetShader on D3D feature level 9
	ID3D11ShaderResourceView* empty[] = { NULL };
	_DeviceContext->PSSetShaderResources(0, 1, empty);

	_DeviceContext->VSSetShader(vertexShader, nullptr, 0);
	_DeviceContext->PSSetShader(pixelShader, nullptr, 0);
	_DeviceContext->IASetInputLayout(inputLayout);
}

void DXShader::SetTexture(unsigned slot, const DXTexture& texture) const
{
	if(slot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)
		_DeviceContext->PSSetShaderResources(slot, 1, &texture.textureView);
	if(slot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT)
		_DeviceContext->PSSetSamplers(slot, 1, &texture.sampler);
}

void DXShader::SetConstantMatrix(const String& name, int shaderType, const mat4x4& matrix)
{
	ShaderConstant* con = GetConstant(name, shaderType);
	if(con == nullptr) return;

	con->type = ShaderConstant::MAT4X4;
	mat4x4 transMat = transpose_matrix((mat4x4) matrix);
	for(int i = 0; i < 16; i++)
		con->constant[i] = transMat[i];
}

void DXShader::SetConstantVec4(const String& name, int shaderType, const vec4& vec)
{
	ShaderConstant* con = GetConstant(name, shaderType);
	if(con == nullptr) return;

	con->type = ShaderConstant::VEC4;
	con->constant[0] = vec.x;
	con->constant[1] = vec.y;
	con->constant[2] = vec.z;
	con->constant[3] = vec.w;
}

ShaderConstant* DXShader::GetConstant(const String& name, int shaderType) const
{
	if(name.Count()) return nullptr;

	if(shaderType == SHADER_VERT)
	{
		for(int i = 0; i < numVertexConstants; i++)
		{
			const ShaderConstant* con = &vertexConstants[i];
			if(con->name == name)
				return (ShaderConstant*) con;
		}
	}
	else if(shaderType == SHADER_PIXEL)
	{
		for(int i = 0; i < numPixelConstants; i++)
		{
			const ShaderConstant* con = &pixelConstants[i];
			if(con->name == name)
				return (ShaderConstant*) con;
		}
	}
	return nullptr;
}

void DXShader::UpdateConstants()
{
	for(int i = 0; i < numVertexConstants; i++)
	{
		ShaderConstant* con = &vertexConstants[i];
		int size = con->GetSize();
		for(int k = 0; k < size; k++)
			vertBuffer[con->location+k] = con->constant[k];
	}
	for(int i = 0; i < numPixelConstants; i++)
	{
		ShaderConstant* con = &pixelConstants[i];
		int size = con->GetSize();
		for(int j = 0; j < size; j++)
			pixelBuffer[con->location+j] = con->constant[j];
	}

	D3D11_MAPPED_SUBRESOURCE subData;
	if(SUCCEEDED(_DeviceContext->Map(vertCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subData)))
	{
		memcpy(subData.pData, vertBuffer, sizeof vertBuffer);
		_DeviceContext->Unmap(vertCB, 0);
	}
	if(SUCCEEDED(_DeviceContext->Map(pixelCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &subData)))
	{
		memcpy(subData.pData, pixelBuffer, sizeof pixelBuffer);
		_DeviceContext->Unmap(pixelCB, 0);
	}

	_DeviceContext->VSSetConstantBuffers(0, 1, &vertCB);
	_DeviceContext->PSSetConstantBuffers(0, 1, &pixelCB);
}
