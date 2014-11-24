#include "DXUtils.h"

#include "../utilities/Unicode.h"

#if 0
#include <DxErr.h>

String dxerr_text(HRESULT hr)
{
	const wchar_t* errText = DXGetErrorString(hr);
	const wchar_t* errDesc = DXGetErrorDescription(hr);

	String output;
	if(errText != nullptr && errDesc != nullptr)
	{
		char* buffer = nullptr;
		utf16_to_utf8(&buffer, (char16_t*) errText);
		output.Append(buffer);
		output.Append(" : ");
		delete[] buffer;

		buffer = nullptr;
		utf16_to_utf8(&buffer, (char16_t*) errDesc);
		output.Append(buffer);
		delete[] buffer;
	}

	return output;
}
#endif

String hresult_text(HRESULT hr)
{
	String output;

	LPWSTR errorText = NULL;
	if(FormatMessageW(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, // unused with FORMAT_MESSAGE_FROM_SYSTEM
		hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR) &errorText,
		0, // minimum size for output buffer
		NULL))
	{
		char* buffer = nullptr;
		utf16_to_utf8(&buffer, (char16_t*)errorText);
		output.Append(buffer);
		delete[] buffer;

		LocalFree(errorText);
		errorText = NULL;
	}
	
	return output;
}

const char* get_feature_level_name(D3D_FEATURE_LEVEL level)
{
	switch(level)
	{
		case D3D_FEATURE_LEVEL_9_1:		return "D3D_FEATURE_LEVEL_9_1";
		case D3D_FEATURE_LEVEL_9_2:		return "D3D_FEATURE_LEVEL_9_2";
		case D3D_FEATURE_LEVEL_9_3:		return "D3D_FEATURE_LEVEL_9_3";
		case D3D_FEATURE_LEVEL_10_0:	return "D3D_FEATURE_LEVEL_10_0";
		case D3D_FEATURE_LEVEL_10_1:	return "D3D_FEATURE_LEVEL_10_1";
		case D3D_FEATURE_LEVEL_11_0:	return "D3D_FEATURE_LEVEL_11_0";
	}
	return "Unknown feature level!";
}

int dx_get_max_tex_dim()
{
	switch(dx_feature_level)
	{
		default:
		case D3D_FEATURE_LEVEL_9_1:
		case D3D_FEATURE_LEVEL_9_2:
			return 2048;
		case D3D_FEATURE_LEVEL_9_3:
			return 4096;
		case D3D_FEATURE_LEVEL_10_0:
		case D3D_FEATURE_LEVEL_10_1:
			return 8192;
		case D3D_FEATURE_LEVEL_11_0:
			return 16384;
	}
}

bool dx_non_power_of_two_cond()
{
	switch(dx_feature_level)
	{
		default:
		case D3D_FEATURE_LEVEL_9_1:
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_3:
			return true;
		case D3D_FEATURE_LEVEL_10_0:
		case D3D_FEATURE_LEVEL_10_1:
		case D3D_FEATURE_LEVEL_11_0:
			return false;
	}
}

int dx_get_max_anisotropy()
{
	switch(dx_feature_level)
	{
		default:
		case D3D_FEATURE_LEVEL_9_1:
			return 2;
		case D3D_FEATURE_LEVEL_9_2:
		case D3D_FEATURE_LEVEL_9_3:
		case D3D_FEATURE_LEVEL_10_0:
		case D3D_FEATURE_LEVEL_10_1:
		case D3D_FEATURE_LEVEL_11_0:
			return 16;
	}
}

size_t size_of_format(DXGI_FORMAT format)
{
    switch(format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
            return 16;
               
        case DXGI_FORMAT_R32G32B32_TYPELESS:
        case DXGI_FORMAT_R32G32B32_FLOAT:
        case DXGI_FORMAT_R32G32B32_UINT:
        case DXGI_FORMAT_R32G32B32_SINT:
            return 12;
               
        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        case DXGI_FORMAT_R32G8X24_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
        case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
            return 8;
       
        case DXGI_FORMAT_R10G10B10A2_TYPELESS:
        case DXGI_FORMAT_R10G10B10A2_UNORM:
        case DXGI_FORMAT_R10G10B10A2_UINT:
        case DXGI_FORMAT_R11G11B10_FLOAT:
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        case DXGI_FORMAT_R24G8_TYPELESS:
        case DXGI_FORMAT_D24_UNORM_S8_UINT:
        case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
        case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
        case DXGI_FORMAT_B8G8R8X8_UNORM:
            return 4;
               
        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        case DXGI_FORMAT_B5G6R5_UNORM:
        case DXGI_FORMAT_B5G5R5A1_UNORM:
            return 2;
               
        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
            return 1;
       
        // Compressed format
        case DXGI_FORMAT_BC2_TYPELESS:
        case DXGI_FORMAT_BC2_UNORM:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_TYPELESS:
        case DXGI_FORMAT_BC3_UNORM:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_BC5_TYPELESS:
        case DXGI_FORMAT_BC5_UNORM:
        case DXGI_FORMAT_BC5_SNORM:
            return 16;
               
        // Compressed format
        case DXGI_FORMAT_R1_UNORM:
        case DXGI_FORMAT_BC1_TYPELESS:
        case DXGI_FORMAT_BC1_UNORM:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC4_TYPELESS:
        case DXGI_FORMAT_BC4_UNORM:
        case DXGI_FORMAT_BC4_SNORM:
            return 8;
       
        // Compressed format
        case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
            return 4;
       
        // These are compressed, but bit-size information is unclear.
        case DXGI_FORMAT_R8G8_B8G8_UNORM:
        case DXGI_FORMAT_G8R8_G8B8_UNORM:
            return 4;

        case DXGI_FORMAT_UNKNOWN:
        default:
            return 0;
    }
	return 0;
}

void get_tex_image(void* data, ID3D11Texture2D* texture)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	texture->GetDesc(&textureDesc);
	textureDesc.Usage = D3D11_USAGE_STAGING;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	textureDesc.BindFlags = 0;

	HRESULT result = S_OK;

	ID3D11Texture2D* stageTexture = NULL;
	result = _Device->CreateTexture2D(&textureDesc, NULL, &stageTexture);
	if(FAILED(result)) return;

	_DeviceContext->CopyResource(stageTexture, texture);

	D3D11_MAPPED_SUBRESOURCE mappedSubResource;
	result = _DeviceContext->Map(stageTexture, 0, D3D11_MAP_READ, 0, &mappedSubResource);
	if(FAILED(result))
	{
		stageTexture->Release();
		return;
	}

	size_t dataWidth = size_of_format(textureDesc.Format) * textureDesc.Width * textureDesc.Height;
	memcpy(data, mappedSubResource.pData, dataWidth);

	_DeviceContext->Unmap(stageTexture, 0);
	stageTexture->Release();
}
