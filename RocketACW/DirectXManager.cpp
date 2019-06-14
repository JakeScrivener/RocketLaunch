#include "DirectXManager.h"
#include "Instance.h"
#include "Result.h"
#include <winerror.h>
#include <d3d11.h>

using namespace DirectX;
using namespace std;

/// <summary>
/// Constructor of the dxManager which calls device initialisation
/// </summary>
/// <param name="pHWnd"> the window to bind dx output to </param>
/// <param name="pHWnd"> a reference to the anttweak manager to use </param>
DirectXManager::DirectXManager(const HWND& pHWnd, AntTweakManager& pAwManager)
{
	mAwManager = &pAwManager;
	if (FAILED(InitDevice(pHWnd)))
	{
		Cleanup();
	}
}

/// <summary>
/// Destructor of for the directx manager
/// </summary>
DirectXManager::~DirectXManager() = default;

/// <summary>
/// Releases the memory from directx pointers
/// </summary>
void DirectXManager::Cleanup()
{
	for (const auto& texture : mTexMap)
	{
		(texture.second)->Release(); // delete texture
	}
	for (const auto& shader : mShaderMap)
	{
		(get<0>(shader.second))->Release(); // delete vertex shader
		(get<1>(shader.second))->Release(); // delete vertex layout
		(get<2>(shader.second))->Release(); // delete pixel shader
	}
	for (const auto& geometry : mGeometryBufferMap)
	{
		(get<0>(geometry.second))->Release(); // delete vertex buffer
		(get<1>(geometry.second))->Release(); // delete index buffer
	}
	for (const auto& instance : mInstanceMap)
	{
		(instance.second)->Release();
	}

	//delete the remaining directx pointers
	if (mAlphaBlend) mAlphaBlend->Release();
	if (mDepthStencilState) mDepthStencilState->Release();
	if (mNoCullRasterizerState) mNoCullRasterizerState->Release();
	if (mDefaultRasterizerState) mDefaultRasterizerState->Release();
	if (mTexSampler) mTexSampler->Release();
	if (mImmediateContext) mImmediateContext->ClearState();
	if (mConstantBuffer) mConstantBuffer->Release();
	if (mConstantBufferUniform) mConstantBufferUniform->Release();
	if (mDepthStencil) mDepthStencil->Release();
	if (mDepthStencilView) mDepthStencilView->Release();
	if (mRenderTargetView) mRenderTargetView->Release();
	if (mSwapChain1) mSwapChain1->Release();
	if (mSwapChain) mSwapChain->Release();
	if (mImmediateContext1) mImmediateContext1->Release();
	if (mImmediateContext) mImmediateContext->Release();
	if (mDevice1) mDevice1->Release();
	if (mDevice) mDevice->Release();
}


/// <summary>
/// Helper to load shaders from file
/// </summary>
/// <param name="pFileName"> shaders file name </param>
/// <param name="pEntryPoint"> name of function that is entry point to shader </param>
/// <param name="pShaderModel"> version of shader </param>
/// <param name="pBlobOut"> </param>
/// <returns> HRESULT of compiling the ahader</returns>
HRESULT DirectXManager::CompileShaderFromFile(const WCHAR * const pFileName, const LPCSTR pEntryPoint, const LPCSTR pShaderModel, ID3DBlob** const pBlobOut)
{
	auto hr{ Result::OK };

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;

	// Disable optimizations to further improve shader debugging
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif



	ID3DBlob* errorBlob = nullptr;
	hr = D3DCompileFromFile(pFileName, nullptr, nullptr, pEntryPoint, pShaderModel,
		dwShaderFlags, 0, pBlobOut, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
		}
		return hr;
	}
	if (errorBlob) errorBlob->Release();

	return hr;
}

/// <summary>
/// Initialise the directx graphics context
/// </summary>
/// <param name="pHWnd"> the window which the device will be bound as a render target </param>
/// <returns> the HRESULT of creating the graphics context </returns>
HRESULT DirectXManager::InitDevice(const HWND& pHWnd)
{
	auto hr{ Result::OK };

	RECT rc;
	GetClientRect(pHWnd, &rc);
	const UINT width = rc.right - rc.left;
	const UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	const UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[]
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	const auto numFeatureLevels = ARRAYSIZE(featureLevels);

	for (const auto driverType : driverTypes)
	{
		hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mImmediateContext);

		if (hr == Result::INVALIDARGS)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &mDevice, &mFeatureLevel, &mImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;



	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	IDXGIFactory1* dxgiFactory = nullptr;
	{
		IDXGIDevice* dxgiDevice = nullptr;
		hr = mDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
		if (SUCCEEDED(hr))
		{
			IDXGIAdapter* adapter = nullptr;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
				adapter->Release();
			}
			dxgiDevice->Release();
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	IDXGIFactory2* dxgiFactory2 = nullptr;
	hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = mDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&mDevice1));
		if (SUCCEEDED(hr))
		{
			mImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&mImmediateContext1));
		}

		DXGI_SWAP_CHAIN_DESC1 sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(mDevice, pHWnd, &sd, nullptr, nullptr, &mSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = mSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&mSwapChain));
		}

		dxgiFactory2->Release();
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = pHWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(mDevice, &sd, &mSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(pHWnd, DXGI_MWA_NO_ALT_ENTER);

	dxgiFactory->Release();

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* backBuffer = nullptr;
	hr = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
		return hr;

	hr = mDevice->CreateRenderTargetView(backBuffer, nullptr, &mRenderTargetView);
	backBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC depthDesc;
	ZeroMemory(&depthDesc, sizeof(depthDesc));
	depthDesc.Width = width;
	depthDesc.Height = height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	hr = mDevice->CreateTexture2D(&depthDesc, nullptr, &mDepthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = depthDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = mDevice->CreateDepthStencilView(mDepthStencil, &depthStencilViewDesc, &mDepthStencilView);
	if (FAILED(hr))
		return hr;

	mImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);

	//Set the sampler
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	hr = mDevice->CreateSamplerState(&samplerDesc, &mTexSampler);
	if (FAILED(hr))
		return hr;

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = static_cast<float>(width);
	vp.Height = static_cast<float>(height);
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	mImmediateContext->RSSetViewports(1, &vp);

	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = mDevice->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
	if (FAILED(hr))
		return hr;

	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_NONE;

	hr = mDevice->CreateRasterizerState(&rasterizerDesc, &mNoCullRasterizerState);
	if (FAILED(hr))
		return hr;

	D3D11_RASTERIZER_DESC rasterizerDesc2;
	ZeroMemory(&rasterizerDesc2, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerDesc2.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc2.CullMode = D3D11_CULL_BACK;

	hr = mDevice->CreateRasterizerState(&rasterizerDesc2, &mDefaultRasterizerState);
	if (FAILED(hr))
		return hr;

	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;

	hr = mDevice->CreateBlendState(&blendDesc, &mAlphaBlend);
	if (FAILED(hr))
		return hr;

	// Set primitive topology
	mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//improved object reporting for finding memory leaks
	//#ifdef _DEBUG
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	ID3D11Debug* DebugDevice = nullptr;
	//	hr = mDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&DebugDevice));
	//
	//	hr = DebugDevice->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	//	if (DebugDevice) DebugDevice->Release();
	//
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//	// -----------------------------------------------------------
	//#endif

	//Create constant buffers
	CreateConstantBuffers();

	mAwManager->Init(mDevice, width, height);

	return hr;
}

/// <summary>
/// Create the constant buffers
/// </summary>
/// <returns> the HRESULT of creating the constant buffers </returns>
HRESULT DirectXManager::CreateConstantBuffers()
{
	auto hr{ Result::OK };
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	// Create the constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBuffer);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, nullptr, &mConstantBuffer);
	if (FAILED(hr))
		return hr;

	//Create uniform constant buffer
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ConstantBufferUniform);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = mDevice->CreateBuffer(&bd, nullptr, &mConstantBufferUniform);
	if (FAILED(hr))
		return hr;

	return hr;
}

/// <summary>
/// creates the vertex buffer and index buffer for the given shape if they dont already exist
/// </summary>
/// <param name="pShape"> the shape which will have its vertices loaded </param>
/// <returns> the HRESULT of creating the vertex buffer </returns>
HRESULT DirectXManager::LoadGeometryBuffers(const Shape & pShape)
{
	auto hr{ Result::OK };
	auto it = mGeometryBufferMap.find(
		pShape.Geometry());
	if (it != mGeometryBufferMap.end())
	{
		// Set vertex buffer
		const UINT stride = sizeof(SimpleVertex);
		const UINT offset = 0;
		mImmediateContext->IASetVertexBuffers(0, 1, &get<0>(it->second), &stride, &offset);

		// Set index buffer
		mImmediateContext->IASetIndexBuffer(get<1>(it->second), DXGI_FORMAT_R16_UINT, 0);
	}
	else
	{
		//Create vertex buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = pShape.Vertices().size() * sizeof(SimpleVertex);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = &(pShape.Vertices()[0]);
		ID3D11Buffer* VertBuffer = nullptr;
		hr = mDevice->CreateBuffer(&bd, &initData, &VertBuffer);
		if (FAILED(hr))
			return hr;

		// Set vertex buffer
		const UINT stride = sizeof(SimpleVertex);
		const UINT offset = 0;
		mImmediateContext->IASetVertexBuffers(0, 1, &VertBuffer, &stride, &offset);

		//Create index buffer
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = pShape.Indices().size() * sizeof(WORD);
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		initData.pSysMem = &(pShape.Indices()[0]);
		ID3D11Buffer* IndBuffer = nullptr;
		hr = mDevice->CreateBuffer(&bd, &initData, &IndBuffer);
		if (FAILED(hr))
			return hr;

		// Set index buffer
		mImmediateContext->IASetIndexBuffer(IndBuffer, DXGI_FORMAT_R16_UINT, 0);

		mGeometryBufferMap.insert(pair<GeometryType, tuple<ID3D11Buffer*, ID3D11Buffer*>>(pShape.Geometry(), make_tuple(VertBuffer, IndBuffer)));
	}


	return hr;
}

/// <summary>
/// Load in the diffuse texture, normal map and height map of the shape
/// </summary>
/// <param name="pShape"> the shape which will have its textures loaded</param>
/// <returns> HRESULT of the texture loads </returns>
HRESULT DirectXManager::LoadTextures(const Shape& pShape)
{
	auto hr{ Result::OK };

	//diffuse texture
	auto it = mTexMap.find(pShape.DiffuseTexture());

	if (it != mTexMap.end())
	{
		mImmediateContext->PSSetShaderResources(0, 1, &(it->second));
	}
	else if (!pShape.DiffuseTexture().empty())
	{
		ID3D11ShaderResourceView* diffTexRv = nullptr;
		hr = CreateDDSTextureFromFile(mDevice, pShape.DiffuseTexture().c_str(), nullptr, &diffTexRv);
		if (FAILED(hr))
			return hr;
		mImmediateContext->PSSetShaderResources(0, 1, &diffTexRv);
		mTexMap.insert(pair<wstring, ID3D11ShaderResourceView*>(pShape.DiffuseTexture(), diffTexRv));
	}

	//normal map
	it = mTexMap.find(pShape.NormalMap());
	if (it != mTexMap.end())
	{
		mImmediateContext->PSSetShaderResources(1, 1, &(it->second));
	}
	else if (!pShape.NormalMap().empty())
	{
		ID3D11ShaderResourceView* normTexRv = nullptr;
		hr = CreateDDSTextureFromFile(mDevice, pShape.NormalMap().c_str(), nullptr, &normTexRv);
		if (FAILED(hr))
			return hr;
		mImmediateContext->PSSetShaderResources(0, 1, &normTexRv);
		mTexMap.insert(pair<wstring, ID3D11ShaderResourceView*>(pShape.NormalMap(), normTexRv));
	}

	//height map
	it = mTexMap.find(pShape.HeightMap());
	if (it != mTexMap.end())
	{
		mImmediateContext->PSSetShaderResources(2, 1, &(it->second));

	}
	else if (!pShape.HeightMap().empty())
	{
		ID3D11ShaderResourceView* heightTexRv = nullptr;
		hr = CreateDDSTextureFromFile(mDevice, pShape.HeightMap().c_str(), nullptr, &heightTexRv);
		if (FAILED(hr))
			return hr;
		mImmediateContext->PSSetShaderResources(0, 1, &heightTexRv);
		mTexMap.insert(pair<wstring, ID3D11ShaderResourceView*>(pShape.HeightMap(), heightTexRv));
	}

	return hr;
}

/// <summary>
/// Loads the vertex shader, pixel/fragment shader and the input layout if they have not already been created
/// </summary>
/// <param name="pShape"> the shape which will have its shaders loaded </param>
/// <returns> the result of creating the shaders </returns>
HRESULT DirectXManager::LoadShaders(const Shape & pShape)
{
	auto hr{ Result::OK };
	auto it = mShaderMap.find(pShape.Shader());

	if (it != mShaderMap.end())
	{
		//it->second = tuple

		//Set vertex shader
		//get<0> = Vertex Shader
		mImmediateContext->VSSetShader(get<0>(it->second), nullptr, 0);
		mImmediateContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

		//Set layout
		//get<1> = Vertex Layout
		mImmediateContext->IASetInputLayout(get<1>(it->second));

		//Set pixel shader
		//get<2> = Pixel Shader
		mImmediateContext->PSSetShader(get<2>(it->second), nullptr, 0);
		mImmediateContext->PSSetConstantBuffers(0, 1, &mConstantBuffer);
		mImmediateContext->PSSetConstantBuffers(1, 1, &mConstantBufferUniform);
	}
	else
	{
		// Compile the vertex shader
		ID3DBlob* VSBlob = nullptr;
		hr = CompileShaderFromFile(pShape.Shader().c_str(), "VS", "vs_4_0", &VSBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return hr;
		}

		// Create the vertex shader
		ID3D11VertexShader* vertShader = nullptr;
		hr = mDevice->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &vertShader);
		if (FAILED(hr))
		{
			VSBlob->Release();
			return hr;
		}

		mImmediateContext->VSSetShader(vertShader, nullptr, 0);
		mImmediateContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
		const auto numElements = ARRAYSIZE(layout);

		// Create the input layout
		ID3D11InputLayout* vertLayout = nullptr;
		hr = mDevice->CreateInputLayout(layout, numElements, VSBlob->GetBufferPointer(),
			VSBlob->GetBufferSize(), &vertLayout);
		VSBlob->Release();
		if (FAILED(hr))
			return hr;

		// Set the input layout
		mImmediateContext->IASetInputLayout(vertLayout);

		ID3DBlob* psBlob = nullptr;
		hr = CompileShaderFromFile(pShape.Shader().c_str(), "PS", "ps_4_0", &psBlob);
		if (FAILED(hr))
		{
			MessageBox(nullptr,
				L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
			return hr;
		}

		// Create the pixel shader
		ID3D11PixelShader* pixelShader = nullptr;
		hr = mDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);
		psBlob->Release();
		if (FAILED(hr))
			return hr;

		mImmediateContext->PSSetShader(pixelShader, nullptr, 0);
		mImmediateContext->PSSetConstantBuffers(0, 1, &mConstantBuffer);
		mImmediateContext->PSSetConstantBuffers(1, 1, &mConstantBufferUniform);

		//Add to the dictionary
		mShaderMap.insert(pair<wstring, tuple<ID3D11VertexShader*, ID3D11InputLayout*, ID3D11PixelShader*>>(pShape.Shader(), make_tuple(vertShader, vertLayout, pixelShader)));
	}

	return hr;
}

/// <summary>
/// Loads the instance buffer from a map or creates a new one if one does not already exist
/// </summary>
/// <param name="pShape"> the shape which will have its instance buffer loaded </param>
HRESULT DirectXManager::LoadInstanceBuffers(const Shape & pShape)
{
	auto hr{ Result::OK };
	const auto it = mInstanceMap.find(pShape.Name());

	if (it != mInstanceMap.end())
	{
		// Set instance buffer
		const UINT stride = sizeof(Instance);
		const UINT offset = 0;
		mImmediateContext->IASetVertexBuffers(1, 1, &it->second, &stride, &offset);
		mImmediateContext->UpdateSubresource(it->second, 0, nullptr, &pShape.Instances()[0], 0, 0);
	}
	else
	{
		//Create new buffer
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = pShape.Instances().size() * sizeof(Instance);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = &(pShape.Instances()[0]);
		ID3D11Buffer* instBuffer = nullptr;
		hr = mDevice->CreateBuffer(&bd, &initData, &instBuffer);
		if (FAILED(hr))
			return hr;

		// Set instance buffer
		const UINT stride = sizeof(Instance);
		const UINT offset = 0;
		mImmediateContext->IASetVertexBuffers(1, 1, &instBuffer, &stride, &offset);

		//Add to the dictionary
		mInstanceMap.insert(pair<string, ID3D11Buffer*>(pShape.Name(), instBuffer));
	}

	return hr;
}

/// <summary>
/// Renders the scene
/// </summary>
/// <param name="pGameObjects"> a list of all gameobjects in the scene </param>
/// <param name="pCam"> the currently active camera </param>
/// <param name="pLights"> a vector of the lights in the scene </param>
/// <param name="pTime"> the time since the game started </param>
HRESULT DirectXManager::Render(const std::vector<GameObject>& pGameObjects, const Camera * const pCam, const vector<Light> & pLights, const float pTime)
{
	auto hr{ Result::OK };
	//
	// Clear the back buffer
	//
	mImmediateContext->ClearRenderTargetView(mRenderTargetView, Colors::CornflowerBlue);

	//
	// Clear the depth buffer to 1.0 (max depth)
	//
	mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	XMFLOAT4X4 initialWorld{};
	XMStoreFloat4x4(&initialWorld, XMMatrixTranspose(XMMatrixIdentity()));

	XMFLOAT4X4 view{};
	XMStoreFloat4x4(&view, XMMatrixTranspose(XMLoadFloat4x4(&pCam->View())));

	XMFLOAT4X4 proj{};
	XMStoreFloat4x4(&proj, XMMatrixTranspose(XMLoadFloat4x4(&pCam->Proj())));

	XMFLOAT4 time{};
	time.x = pTime;
	time.y = pTime;
	time.z = pTime;
	time.w = pTime;

	ConstantBuffer cb1{
		initialWorld,
		view,
		proj,
		pCam->Eye(),
		time
	};

	ConstantBufferUniform cbu{};

	for (auto i = 0; i < pLights.size(); ++i)
	{
		cbu.mLightPosition[i] = pLights[i].Position();
		cbu.mLightColour[i] = pLights[i].Colour();
	}
	cbu.mNumberOfLights.x = pLights.size();
	cbu.mNumberOfLights.y = pLights.size();
	cbu.mNumberOfLights.z = pLights.size();
	cbu.mNumberOfLights.w = pLights.size();

	mImmediateContext->UpdateSubresource(mConstantBufferUniform, 0, nullptr, &cbu, 0, 0);
	mImmediateContext->PSSetSamplers(0, 1, &mTexSampler);

	//Loop through each gameobject
	for (const auto& gameObject : pGameObjects)
	{
		//Loop through each shape in the gameobject
		for (const auto& shape : gameObject.Shapes())
		{
			hr = LoadGeometryBuffers(shape);
			if (FAILED(hr))
				return hr;

			hr = LoadTextures(shape);
			if (FAILED(hr))
				return hr;

			hr = LoadShaders(shape);
			if (FAILED(hr))
				return hr;


			//Get data from the shape and set it in the constant buffer
			XMFLOAT4X4 world{};
			XMStoreFloat4x4(&world, XMMatrixIdentity() * XMLoadFloat4x4(shape.Transform()) * XMLoadFloat4x4(gameObject.Transform()));
			XMStoreFloat4x4(&cb1.mCbWorld, XMMatrixTranspose(XMLoadFloat4x4(&world)));

			mImmediateContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb1, 0, 0);

			if (shape.IsBlended())
			{
				float blendFactor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
				const auto blendSample = 0xffffffff;
				mImmediateContext->OMSetBlendState(mAlphaBlend, blendFactor, blendSample);
				mImmediateContext->OMSetDepthStencilState(mDepthStencilState, 0);
				//mImmediateContext->RSSetState(mNoCullRasterizerState);
			}
			else if (shape.IsEnvironment())
			{
				mImmediateContext->OMSetDepthStencilState(mDepthStencilState, 0);
				mImmediateContext->RSSetState(mNoCullRasterizerState);
			}
			else
			{
				const auto blendSample = 0xffffffff;
				mImmediateContext->OMSetBlendState(nullptr, nullptr, blendSample);
				mImmediateContext->OMSetDepthStencilState(nullptr, 0);
				mImmediateContext->RSSetState(mDefaultRasterizerState);
			}
			
			//check if the shape has instancing enabled
			if (!shape.Instances().empty())
			{
				LoadInstanceBuffers(shape);
				//draw instances
				mImmediateContext->DrawIndexedInstanced(shape.Indices().size(), shape.Instances().size(), 0, 0, 0);
			}
			else
			{
				//draw the shape
				mImmediateContext->DrawIndexed(shape.Indices().size(), 0, 0);
			}
		}
	}

	mAwManager->DrawBars();
	//
	// Present our back buffer to our front buffer
	//
	mSwapChain->Present(1, 0);

	return hr;
}