#include "DXTexture.h"

#include "DXUtils.h"

#include "../utilities/stb_image.h"
#include "../utilities/Logging.h"
#include "../utilities/Maths.h"
#include "../utilities/BitManipulation.h"

DXTexture::TextureRecord DXTexture::loadedTextures[MAX_NUM_TEXTURES];

void DXTexture::Initialize()
{
	for(int i = 0; i < MAX_NUM_TEXTURES; i++)
	{
		loadedTextures[i].record = nullptr;
		loadedTextures[i].numOwners = 0;
	}
}

void DXTexture::Terminate()
{
}

DXTexture::DXTexture()
{
	SetDefaults();
}

void DXTexture::SetDefaults()
{
	texture = nullptr;
	textureView = nullptr;
	sampler = nullptr;
	width = height = 0;
}

bool DXTexture::Load(const String& fileName)
{
	DXTexture* existingTexture = nullptr;
	for(int i = 0; i < MAX_NUM_TEXTURES; i++)
	{
		DXTexture* tex = loadedTextures[i].record;
		if(tex == nullptr) continue;

		if(loadedTextures[i].fileName == fileName)
		{
			existingTexture = tex;

			width = tex->width;
			height = tex->height;
			texture = tex->texture;
			textureView = tex->textureView;
			sampler = tex->sampler;

			loadedTextures[i].numOwners++;
			break;
		}
	}

	if(existingTexture == nullptr)
	{
		bool loadSuccess = LoadTextureData(fileName);
		if(loadSuccess)
		{
			for(int i = 0; i < MAX_NUM_TEXTURES; i++)
			{
				DXTexture* tex = loadedTextures[i].record;
				if(tex != nullptr) continue;

				loadedTextures[i].fileName = fileName;
				loadedTextures[i].record = this;
				loadedTextures[i].numOwners = 1;
				break;
			}
		}
		return loadSuccess;
	}
	return true;
}

bool DXTexture::LoadTextureData(const String& fileName)
{
	String errorText("Error loading file ");
	errorText.Append(fileName);
	errorText.Append("! -> ");

	String path("data/textures/");
	path.Append(fileName);

	FILE* file = fopen(path.Data(), "rb");
	if(file == nullptr)
	{
		LOG_ISSUE("%s file could not be opened", errorText.Data());
		fclose(file);
		return false;
	}

	int imageWidth, imageHeight, numComponents;
	unsigned char* data = stbi_load_from_file(file, &imageWidth, &imageHeight, &numComponents, 4);
	if(data == nullptr)
	{
		LOG_ISSUE("%s -STB IMAGE ERROR: %s", errorText.Data(), stbi_failure_reason());
		fclose(file);
		return false;
	}

	fclose(file);

	int maxTextureSize = dx_get_max_tex_dim();
	if(imageWidth > maxTextureSize) imageWidth = maxTextureSize;
	if(imageHeight > maxTextureSize) imageHeight = maxTextureSize;

	if(dx_non_power_of_two_cond())
	{
		imageWidth = get_next_power_of_two(imageWidth);
		imageHeight = get_next_power_of_two(imageHeight);
	}

	width = imageWidth;
	height = imageHeight;

	int bpp = numComponents * 8;
	int pitch = numComponents * imageWidth;
	bool loaded = LoadDataToDX(data, bpp, pitch, errorText);

	stbi_image_free(data);

	return loaded;
}

bool DXTexture::LoadDataToDX(BYTE* data, int bpp, int pitch, const String& errorText)
{
	HRESULT hr = S_OK;

	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	if(bpp == 24) format = DXGI_FORMAT_B8G8R8A8_UNORM;
	if(bpp == 32) format = DXGI_FORMAT_B8G8R8A8_UNORM;
		
	if(format == DXGI_FORMAT_UNKNOWN)
	{
		LOG_ISSUE("%s DIRECTX ERROR: %s bit format of image not supported!",
			errorText.Data(), hresult_text(hr).Data());
		return false;
	}

	UINT numMipmaps = binary_log((width < height)? width : height);

	D3D11_TEXTURE2D_DESC textureDesc = {0};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.Format = format;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
	textureDesc.MipLevels = numMipmaps;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
 
	hr = _Device->CreateTexture2D(&textureDesc, NULL, &texture);
	if(FAILED(hr))
	{
		LOG_ISSUE("%s DIRECTX ERROR: %s texture could not be created!",
			errorText.Data(), hresult_text(hr).Data());
		return false;
	}
	_DeviceContext->UpdateSubresource(texture, 0, NULL, data, pitch, 0);
 
	D3D11_SHADER_RESOURCE_VIEW_DESC textureViewDesc;
	ZeroMemory(&textureViewDesc, sizeof(textureViewDesc));
	textureViewDesc.Format = textureDesc.Format;
	textureViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	textureViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
	textureViewDesc.Texture2D.MostDetailedMip = 0;

	hr = _Device->CreateShaderResourceView(texture, &textureViewDesc, &textureView);
	if(FAILED(hr))
	{
		LOG_ISSUE("%s DIRECTX ERROR: %s shader resource view could not be made",
			errorText.Data(), hresult_text(hr).Data());
		return false;
	}
	_DeviceContext->GenerateMips(textureView);
 
	float anisotropy = 16.0f;
	float maxAnisotropy = dx_get_max_anisotropy();
	if(anisotropy > maxAnisotropy)
		anisotropy = maxAnisotropy;

	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = (anisotropy >= 1.0f) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.MaxAnisotropy = anisotropy;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;

	hr = _Device->CreateSamplerState(&samplerDesc, &sampler);
	if(FAILED(hr))
	{
		LOG_ISSUE("%s DIRECTX ERROR: %s sampler could not be made for texture",
			errorText.Data(), hresult_text(hr).Data());
		return false;
	}
	return true;
}

void DXTexture::Unload()
{
	for(int i = 0; i < MAX_NUM_TEXTURES; i++)
	{
		DXTexture* tex = loadedTextures[i].record;
		if(tex == nullptr)
			continue;

		if(loadedTextures[i].numOwners <= 1)
		{
			texture->Release();
			textureView->Release();
			sampler->Release();

			loadedTextures[i].numOwners = 0;
			loadedTextures[i].record = nullptr;
			loadedTextures[i].fileName.Clear();
		}
		else
			loadedTextures[i].numOwners--;
		break;
	}
	SetDefaults();
}
