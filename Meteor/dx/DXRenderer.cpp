#include "DXRenderer.h"

#include "DXUtils.h"
#include "DXShader.h"
#include "DXModel.h"
#include "DXPrimitives.h"

#include "utilities/Macros.h"
#include "utilities/Logging.h"

#include "CameraData.h"

#if _DEBUG
#include <D3D11SDKLayers.h>
#endif

#include <sstream>

D3D_FEATURE_LEVEL dx_feature_level;
ID3D11DeviceContext* _DeviceContext;
ID3D11Device* _Device;

using namespace std;

enum BlendState
{
	BLEND_ALPHA,
	BLEND_SOLID,
};

enum DepthState
{
	DEPTH_RW,
	DEPTH_RO,
	DEPTH_OFF,
};

enum ViewportLayer
{
	LAYER_WORLD,
	LAYER_SCREEN,
	LAYER_GUI,
};

namespace DXRenderer
{
	IDXGISwapChain* swapChain;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;

	ID3D11BlendState *alphaBlend, *solidBlend;
	ID3D11RasterizerState* rasterizerState;
	ID3D11DepthStencilState *defaultDepthState, *readOnlyDepthState, *offDepthState, 
		*defaultOrthoDepthState, *readOnlyOrthoDepthState, *offOrthoDepthState;

	ID3D11Query* deviceQueryPipeline[2];
	ID3D11Query* deviceQueryBeginTimestamp[2];
	ID3D11Query* deviceQueryEndTimestamp[2];
	ID3D11Query* deviceQueryDisjoint[2];

	bool enableCaptureRenderStatistics = false;
	wstring queryPipelineText, queryTimestampText;
	int profilerFrameQuery = 0;

	bool enableVerticalSynchronization = false;

	int width, height;

	CameraData cameraData;

	DXShader defaultShader, alphaTestShader;
	DXTexture blankTexture;
	DXModel wonk;

	bool InitSwapChain(IDXGIDevice* dxgiDevice, HWND hWnd, bool isFullscreen);
	bool CreateSwapChain(IDXGIAdapter* dxgiAdapter, HWND hWnd, bool isFullscreen);
	void RenderScene();
	void RenderScreen();
	void ProfilingBegin();
	void ProfilingEnd();
	void ClearScreen(bool isOrtho);
	void SetDepthState(DepthState state, bool isOrtho);
	void SetBlendState(BlendState state);

	void DXError(HRESULT hr, const char* errorString);
}

bool DXRenderer::Initialize(HWND hWnd, bool isFullscreen, bool enableDebugging)
{
	HRESULT hr = S_OK;

	//Create Device and Context
	ID3D11Device* d3dDevice;
	ID3D11DeviceContext* d3dDeviceContext;

	D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1,
    };

	// This flag is required in order to enable compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

	#if _DEBUG
	// enable debugging via SDK Layers with this flag.
	if(enableDebugging)
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	#endif

	hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE, 
        nullptr,                    // leave as nullptr if hardware is used 
        creationFlags,              // optionally set debug and Direct2D compatibility flags 
        featureLevels, 
        ARRAYSIZE(featureLevels), 
        D3D11_SDK_VERSION,          // always set this to D3D11_SDK_VERSION 
        &d3dDevice,
        &dx_feature_level,
        &d3dDeviceContext);
	if(FAILED(hr))
	{
		DXError(hr, "failed to create device!");
		return false;
	}

	_Device = d3dDevice;
    _DeviceContext = d3dDeviceContext;

	// get device interface and create swap chain
	IDXGIDevice* dxgiDevice;
	hr = _Device->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
	if(FAILED(hr))
	{
		DXError(hr, "failed to retrieve interface IDXGIDevice from device");
		return false;
	}

	InitSwapChain(dxgiDevice, hWnd, isFullscreen);

	dxgiDevice->Release();

	// set fullscreen state for swap chain
	hr = swapChain->SetFullscreenState(isFullscreen, NULL);
	if(FAILED(hr))
	{
		DXError(hr, "could not set fullscreen state");
		return false;
	}

	//Create Blend States
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.IndependentBlendEnable = false;
	blendDesc.RenderTarget[0].BlendEnable = true;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO; 
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = _Device->CreateBlendState(&blendDesc, &alphaBlend);
	if(FAILED(hr))
	{
		DXError(hr, "could not create transparent blend state");
		return false;
	}

	D3D11_BLEND_DESC solidDesc;
	ZeroMemory(&solidDesc, sizeof(D3D11_BLEND_DESC));
	solidDesc.RenderTarget[0].BlendEnable = FALSE;
	solidDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = _Device->CreateBlendState(&solidDesc, &solidBlend);
	if(FAILED(hr))
	{
		DXError(hr, "could not create solid blend state");
		return false;
	}

	//Create Rasterizer States
	D3D11_RASTERIZER_DESC rasterDesc;
	ZeroMemory(&rasterDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthClipEnable = TRUE;
	hr = _Device->CreateRasterizerState(&rasterDesc, &rasterizerState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create rasterizer state");
		return false;
	}
	_DeviceContext->RSSetState(rasterizerState);

	//Create Depth Stencil States
	//	defaultDepthState
	D3D11_DEPTH_STENCIL_DESC defaultDepthDesc;
	ZeroMemory(&defaultDepthDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	defaultDepthDesc.DepthEnable = TRUE;
	defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	defaultDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	defaultDepthDesc.StencilEnable = FALSE;
	defaultDepthDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	defaultDepthDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	defaultDepthDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	defaultDepthDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	defaultDepthDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	defaultDepthDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	defaultDepthDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	defaultDepthDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	defaultDepthDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	defaultDepthDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	hr = _Device->CreateDepthStencilState(&defaultDepthDesc, &defaultDepthState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create default depth stencil state");
		return false;
	}
	//	readOnlyDepthState
	defaultDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = _Device->CreateDepthStencilState(&defaultDepthDesc, &readOnlyDepthState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create read only depth stencil state");
		return false;
	}
	//	offDepthState
	defaultDepthDesc.DepthEnable = FALSE;
	defaultDepthDesc.DepthFunc = D3D11_COMPARISON_LESS;
	defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = _Device->CreateDepthStencilState(&defaultDepthDesc, &offDepthState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create off depth stencil state");
		return false;
	}
	//	defaultOrthoDepthState
	defaultDepthDesc.DepthEnable = TRUE;
	defaultDepthDesc.DepthFunc = D3D11_COMPARISON_GREATER;
	defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	hr = _Device->CreateDepthStencilState(&defaultDepthDesc, &defaultOrthoDepthState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create default orthographic depth stencil state");
		return false;
	}
	//	readOnlyOrthoDepthState
	defaultDepthDesc.DepthFunc = D3D11_COMPARISON_GREATER;
	defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = _Device->CreateDepthStencilState(&defaultDepthDesc, &readOnlyOrthoDepthState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create read only orthographic depth stencil state");
		return false;
	}
	//	offOrthoDepthState
	defaultDepthDesc.DepthEnable = FALSE;
	defaultDepthDesc.DepthFunc = D3D11_COMPARISON_GREATER;
	defaultDepthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	hr = _Device->CreateDepthStencilState(&defaultDepthDesc, &offOrthoDepthState);
	if(FAILED(hr))
	{
		DXError(hr, "could not create off orthographic depth stencil state");
		return false;
	}

	// create profiling query objects
	enableCaptureRenderStatistics = enableDebugging;
	if(enableCaptureRenderStatistics)
	{
		D3D11_QUERY_DESC queryDesc = {};
		queryDesc.MiscFlags = 0;
		for(int i = 0; i < 2; i++)
		{
			queryDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
			hr = d3dDevice->CreateQuery(&queryDesc, &deviceQueryDisjoint[i]);
			if(FAILED(hr))
			{
				Log::Add(Log::INFO, "%s%s%s", "DIRECTX: ", dxerr_text(hr), " - failed to create timestamp disjoint query object");
			}

			queryDesc.Query = D3D11_QUERY_TIMESTAMP;
			hr = d3dDevice->CreateQuery(&queryDesc, &deviceQueryBeginTimestamp[i]);
			if(FAILED(hr))
			{
				Log::Add(Log::INFO, "%s%s%s", "DIRECTX: ", dxerr_text(hr), " - failed to create beginning-of-frame timestamp query object");
			}

			queryDesc.Query = D3D11_QUERY_TIMESTAMP;
			hr = d3dDevice->CreateQuery(&queryDesc, &deviceQueryEndTimestamp[i]);
			if(FAILED(hr))
			{
				Log::Add(Log::INFO, "%s%s%s", "DIRECTX: ", dxerr_text(hr), " - failed to create end-of-frame timestamp query object");
			}

			queryDesc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
			hr = d3dDevice->CreateQuery(&queryDesc, &deviceQueryPipeline[i]);
			if(FAILED(hr))
			{
				Log::Add(Log::INFO, "%s%s%s", "DIRECTX: ", dxerr_text(hr), " - failed to create pipeline statistics query object");
			}
		}
	}

	// init textures
	DXTexture::Initialize();

	blankTexture.Load("tex/blank.png");

	// setup shaders
	defaultShader.Load("default_vertex.cso", "default_pixel.cso");
	alphaTestShader.Load("default_vertex.cso", "alpha_test.cso");

	const vec4 multiplyColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
	defaultShader.SetConstantVec4("color", SHADER_PIXEL, multiplyColor);
	alphaTestShader.SetConstantVec4("color", SHADER_PIXEL, multiplyColor);

	// init models
	wonk.LoadAsMesh("meshes/Fiona.obj");

	cameraData.projection = MAT_I;
	cameraData.isOrtho = false;

	DXPrimitives::Initialize();

	return true;
}

bool DXRenderer::InitSwapChain(IDXGIDevice* dxgiDevice, HWND hWnd, bool isFullscreen)
{
	HRESULT hr = S_OK;

	IDXGIAdapter* dxgiAdapter;
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	if(FAILED(hr))
	{
		DXError(hr, "could not retrieve adapter from IDXGIDevice");
		return false;
	}

	bool gotSwapChain = CreateSwapChain(dxgiAdapter, hWnd, isFullscreen);

	dxgiAdapter->Release();
	return gotSwapChain;
}

bool DXRenderer::CreateSwapChain(IDXGIAdapter* dxgiAdapter, HWND hWnd, bool isFullscreen)
{
	HRESULT hr = S_OK;

	IDXGIFactory* dxgiFactory;
	hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
	if(FAILED(hr))
	{
		DXError(hr, "could not retrieve parent factory IDXGIFactory from IDXGIDevice");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {0};
 
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.Flags = 0;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = !isFullscreen;
 
	// Use automatic sizing.
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;

	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
 
	// Don't use multi-sampling.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
 
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_SEQUENTIAL;

	hr = dxgiFactory->CreateSwapChain(_Device, &swapChainDesc, &swapChain);
	if(FAILED(hr))
	{
		DXError(hr, "failed to create swap chain!");
		return false;
	}

	hr = dxgiFactory->MakeWindowAssociation(hWnd, NULL);
	if(FAILED(hr))
	{
		DXError(hr, "failed to attach new swap chain to window!");
		return false;
	}

	dxgiFactory->Release();
	return true;
}

void DXRenderer::Terminate()
{
	DXPrimitives::Terminate();

	wonk.Unload();

	defaultShader.Unload();
	alphaTestShader.Unload();

	blankTexture.Unload();

	DXTexture::Terminate();

	alphaBlend->Release();
	solidBlend->Release();

	rasterizerState->Release();

	defaultDepthState->Release();
	readOnlyDepthState->Release();
	offDepthState->Release();
	defaultOrthoDepthState->Release();
	readOnlyOrthoDepthState->Release();
	offOrthoDepthState->Release();

	swapChain->Release();
	depthStencilView->Release();
	renderTargetView->Release();
	
	if(enableCaptureRenderStatistics)
	{
		for(int i = 0; i < 2; i++)
		{
			deviceQueryPipeline[i]->Release();
			deviceQueryBeginTimestamp[i]->Release();
			deviceQueryEndTimestamp[i]->Release();
			deviceQueryDisjoint[i]->Release();
		}
	}

	/*
	ID3D11Debug* debug;
	_Device->QueryInterface(__uuidof(ID3D11Debug), (void**) &debug);
	debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	debug->Release();
	*/

	_Device->Release();
	_DeviceContext->Release();
}

void DXRenderer::Resize(int dimX, int dimY)
{
	width = dimX;
	height = dimY;

	HRESULT hr = S_OK;

	if (swapChain != nullptr)
	{
		if(depthStencilView != nullptr)
		{
			depthStencilView->Release();
			depthStencilView = nullptr;
		}
		if(renderTargetView != nullptr)
		{
			renderTargetView->Release();
			renderTargetView = nullptr;
		}
		hr = swapChain->ResizeBuffers(2, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
		if(FAILED(hr))
		{
			DXError(hr, "failed to resize swap chain");
			return;
		}
	}

	ID3D11Texture2D* backBuffer;
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if(FAILED(hr))
	{
		DXError(hr, "could not get back buffer from swap chain during resize");
		return;
	}
 
	hr = _Device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
	backBuffer->Release();
	if(FAILED(hr))
	{
		DXError(hr, "failed to create new render target view on resize");
		return;
	}
 
	D3D11_TEXTURE2D_DESC backBufferDesc = {0};
	backBuffer->GetDesc(&backBufferDesc);

	D3D11_TEXTURE2D_DESC depthStencilDesc; 
	depthStencilDesc.Width = backBufferDesc.Width; 
	depthStencilDesc.Height = backBufferDesc.Height; 
	depthStencilDesc.MipLevels = 1; 
	depthStencilDesc.ArraySize = 1; 
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; 
	depthStencilDesc.SampleDesc.Count = 1; 
	depthStencilDesc.SampleDesc.Quality = 0; 
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT; 
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL; 
	depthStencilDesc.CPUAccessFlags = 0; 
	depthStencilDesc.MiscFlags = 0; 

	ID3D11Texture2D* depthStencil; 
	hr = _Device->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencil);
	if(FAILED(hr))
	{
		DXError(hr, "failed to create depth stencil texture during resize");
		return;
	}
 
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc; 
	depthStencilViewDesc.Format = depthStencilDesc.Format; 
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; 
	depthStencilViewDesc.Flags = 0; 
	depthStencilViewDesc.Texture2D.MipSlice = 0; 
	hr = _Device->CreateDepthStencilView(depthStencil, &depthStencilViewDesc, &depthStencilView);
	depthStencil->Release();
	if(FAILED(hr))
	{
		DXError(hr, "failed to create depth stencil view during resize");
		return;
	}
 
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = static_cast<float>(backBufferDesc.Width);
	viewport.Height = static_cast<float>(backBufferDesc.Height);
	viewport.MinDepth = D3D11_MIN_DEPTH;
	viewport.MaxDepth = D3D11_MAX_DEPTH;
	_DeviceContext->RSSetViewports(1, &viewport);
}

void DXRenderer::SetVSync(bool enable)
{
	enableVerticalSynchronization = enable;
}

void DXRenderer::SetCameraState(mat4x4 view, mat4x4 projection, bool isOrthographic)
{
	cameraData.projection = projection;
	cameraData.isOrtho = isOrthographic;
}

void DXRenderer::Render()
{
	if(enableCaptureRenderStatistics)
		ProfilingBegin();

	_DeviceContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

	RenderScene();
	RenderScreen();

	swapChain->Present(enableVerticalSynchronization ? 1 : 0, 0);
	
	if(enableCaptureRenderStatistics)
		ProfilingEnd();
}

void DXRenderer::RenderScene()
{
	const mat4x4 view = view_matrix(cameraData.viewX, cameraData.viewY, cameraData.viewZ, cameraData.position);
	const mat4x4 projection = cameraData.projection;
	const bool isOrtho = cameraData.isOrtho;
	
	ClearScreen(isOrtho);
	SetDepthState(DEPTH_RW, isOrtho);
	SetBlendState(BLEND_SOLID);

	//*** SOLID PASS ***//
	defaultShader.Bind();
	defaultShader.SetConstantMatrix("model", SHADER_VERT, MAT_I);
	defaultShader.SetConstantMatrix("view", SHADER_VERT, view);
	defaultShader.SetConstantMatrix("projection", SHADER_VERT, projection);
	
	wonk.Draw(PHASE_SOLID, defaultShader);

	//*** MASKED PASS ***//
	alphaTestShader.Bind();
	alphaTestShader.SetConstantMatrix("model", SHADER_VERT, MAT_I);
	alphaTestShader.SetConstantMatrix("view", SHADER_VERT, view);
	alphaTestShader.SetConstantMatrix("projection", SHADER_VERT, projection);

	//*** SKY PASS ***//
	SetDepthState(DEPTH_RO, isOrtho);

	defaultShader.Bind();
	defaultShader.SetConstantMatrix("model", SHADER_VERT, MAT_I);
	defaultShader.SetConstantMatrix("view", SHADER_VERT, MAT_I);
	defaultShader.SetConstantMatrix("projection", SHADER_VERT, MAT_I);
	defaultShader.SetConstantVec4("color", SHADER_PIXEL, VEC4_ONE);
	defaultShader.UpdateConstants();
	defaultShader.SetTexture(0, blankTexture);

	const float farthest = (isOrtho) ? 0.000001f : 0.999999f;
	const vec4 skyVerts[] =
	{
		vec4(-1, -1, farthest, 1),
		vec4(1, -1, farthest, 1),
		vec4(-1, 1, farthest, 1),
		vec4(-1, 1, farthest, 1),
		vec4(1, -1, farthest, 1),
		vec4(1, 1, farthest, 1),
	};
	const vec2 skyCoords[] =
	{
		vec2(0.0f, 0.0f),
		vec2(1.0f, 0.0f),
		vec2(0.0f, 1.0f),
		vec2(0.0f, 1.0f),
		vec2(1.0f, 0.0f),
		vec2(1.0f, 1.0f),
	};
	DXPrimitives::DrawTris(skyVerts, skyCoords, ARRAY_LENGTH(skyVerts));

	//*** TRANSPARENT PASS ***//
	SetDepthState(DEPTH_RO, isOrtho);
	SetBlendState(BLEND_ALPHA);

	defaultShader.SetConstantMatrix("model", SHADER_VERT, MAT_I);
	defaultShader.SetConstantMatrix("view", SHADER_VERT, view);
	defaultShader.SetConstantMatrix("projection", SHADER_VERT, projection);

	//*** GUI WORLD PASS ***//
}

void DXRenderer::RenderScreen()
{
	const mat4x4 screenProjection = orthogonal_projection_matrix(0, width, 0, height, -2.0f * height, 0);
	const mat4x4 screenView = view_matrix(UNIT_X, UNIT_Y, UNIT_Z, VEC3_ZERO);

	defaultShader.SetConstantMatrix("view", SHADER_VERT, screenView);
	defaultShader.SetConstantMatrix("projection", SHADER_VERT, screenProjection);
	defaultShader.SetConstantMatrix("model", SHADER_VERT, MAT_I);

	//*** GUI SCREEN PASS ***//
	bool isOrtho = cameraData.isOrtho;
	SetDepthState(DEPTH_OFF, isOrtho);
}

void DXRenderer::ProfilingBegin()
{
	_DeviceContext->Begin(deviceQueryDisjoint[profilerFrameQuery]);
	_DeviceContext->End(deviceQueryBeginTimestamp[profilerFrameQuery]);
	_DeviceContext->Begin(deviceQueryPipeline[profilerFrameQuery]);
}

void DXRenderer::ProfilingEnd()
{
	_DeviceContext->End(deviceQueryEndTimestamp[profilerFrameQuery]);
	_DeviceContext->End(deviceQueryDisjoint[profilerFrameQuery]);
	_DeviceContext->End(deviceQueryPipeline[profilerFrameQuery]);

	queryPipelineText.clear();
	queryTimestampText.clear();
	wostringstream stringStream;

	while(_DeviceContext->GetData(deviceQueryPipeline[profilerFrameQuery], NULL, 0, 0) == S_FALSE);

	D3D11_QUERY_DATA_PIPELINE_STATISTICS pipeStats = {0};
	HRESULT hr = _DeviceContext->GetData(deviceQueryPipeline[profilerFrameQuery], &pipeStats, sizeof(pipeStats), 0);
	if(SUCCEEDED(hr))
	{
		stringStream << L"CInv:" << pipeStats.CInvocations;
		stringStream << L" CPrim:" << pipeStats.CPrimitives;
		queryPipelineText = stringStream.str();
	}

	while (_DeviceContext->GetData(deviceQueryDisjoint[profilerFrameQuery], NULL, 0, 0) == S_FALSE);

	D3D10_QUERY_DATA_TIMESTAMP_DISJOINT disjoint;
	HRESULT hrDis = _DeviceContext->GetData(deviceQueryDisjoint[profilerFrameQuery], &disjoint, sizeof(disjoint), 0);

	UINT64 beginStamp, endStamp;
	HRESULT hrBeg = _DeviceContext->GetData(deviceQueryBeginTimestamp[profilerFrameQuery], &beginStamp, sizeof(beginStamp), 0);
	HRESULT hrEnd = _DeviceContext->GetData(deviceQueryEndTimestamp[profilerFrameQuery], &endStamp, sizeof(endStamp), 0);
			
	if(SUCCEEDED(hrDis) && SUCCEEDED(hrBeg) && SUCCEEDED(hrEnd))
	{
		stringStream.str(L"");
		if(disjoint.Disjoint)
		{
			stringStream << L"Timestamp is disjoint";
		}
		else
		{
			float gpuTime = static_cast<float>(endStamp - beginStamp) / 
							static_cast<float>(disjoint.Frequency) * 1000.0f;
			stringStream << L"GPU Time:" << gpuTime << L"ms";
		}
		queryTimestampText = stringStream.str();
	}
	++profilerFrameQuery &= 1;
}

void DXRenderer::ClearScreen(bool isOrtho)
{
	const float clearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	_DeviceContext->ClearRenderTargetView(renderTargetView, clearColor);

	const float clearDepth = (isOrtho) ? 0.0f : 1.0f;
	_DeviceContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, clearDepth, 0);
}

void DXRenderer::SetDepthState(DepthState state, bool isOrtho)
{
	ID3D11DepthStencilState* depthState = NULL;
	switch (state)
	{
		case DEPTH_RW:
			depthState = (isOrtho) ? defaultOrthoDepthState : defaultDepthState; break;
		case DEPTH_RO:
			depthState = (isOrtho) ? readOnlyOrthoDepthState : readOnlyDepthState; break;
		case DEPTH_OFF:
			depthState = (isOrtho) ? offOrthoDepthState : offDepthState; break;
	}
	_DeviceContext->OMSetDepthStencilState(depthState, 1);
}

void DXRenderer::SetBlendState(BlendState state)
{
	switch(state)
	{
		case BLEND_ALPHA:
			_DeviceContext->OMSetBlendState(alphaBlend, NULL, 0xffffffff); break;
		case BLEND_SOLID:
			_DeviceContext->OMSetBlendState(solidBlend, NULL, 0xffffffff); break;
	}
}

void DXRenderer::DXError(HRESULT hr, const char* errorString)
{
	Log::Add(Log::ERR, "%s%s%s%s", "DIRECTX ERROR: ", dxerr_text(hr), " - ", errorString);
}
