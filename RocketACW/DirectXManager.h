#pragma once
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "Camera.h"
#include "GameObject.h"
#include "Light.h"
#include <map>
#include "AntTweakManager.h"

class DirectXManager
{
	struct ConstantBuffer
	{
		DirectX::XMFLOAT4X4 mCbWorld;
		DirectX::XMFLOAT4X4 mCbView;
		DirectX::XMFLOAT4X4 mCbProjection;
		DirectX::XMFLOAT4 mCbCameraPosition;
		DirectX::XMFLOAT4 mTime;
	};

	struct ConstantBufferUniform
	{
		DirectX::XMFLOAT4 mLightPosition[5];
		DirectX::XMFLOAT4 mLightColour[5];
		DirectX::XMUINT4 mNumberOfLights;
	};

	std::map<std::wstring, ID3D11ShaderResourceView*> mTexMap; //	texture name - texture buffer
	std::map<std::wstring, std::tuple<ID3D11VertexShader*, ID3D11InputLayout*, ID3D11PixelShader*>> mShaderMap; //	shader name - <Vertex Shader, Input Layout, Pixel Shader>
	std::map<GeometryType, std::tuple<ID3D11Buffer*, ID3D11Buffer*>> mGeometryBufferMap; //	geometry type - <Vertices, Indices>
	std::map<std::string, ID3D11Buffer*> mInstanceMap; //	shape name - instance buffer

	D3D_DRIVER_TYPE				mDriverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL			mFeatureLevel = D3D_FEATURE_LEVEL_11_0;
	ID3D11Device*				mDevice = nullptr;
	ID3D11Device1*				mDevice1 = nullptr;
	ID3D11DeviceContext*		mImmediateContext = nullptr;
	ID3D11DeviceContext1*		mImmediateContext1 = nullptr;
	IDXGISwapChain*				mSwapChain = nullptr;
	IDXGISwapChain1*			mSwapChain1 = nullptr;
	ID3D11RenderTargetView*		mRenderTargetView = nullptr;
	ID3D11Texture2D*			mDepthStencil = nullptr;
	ID3D11DepthStencilView*		mDepthStencilView = nullptr;
	ID3D11DepthStencilState*	mDepthStencilState = nullptr;
	ID3D11Buffer*				mConstantBuffer = nullptr;
	ID3D11Buffer*				mConstantBufferUniform = nullptr;
	ID3D11SamplerState*			mTexSampler = nullptr;
	ID3D11RasterizerState*		mNoCullRasterizerState = nullptr;
	ID3D11RasterizerState*		mDefaultRasterizerState = nullptr;
	ID3D11BlendState*			mAlphaBlend = nullptr;

	AntTweakManager* mAwManager;

	static HRESULT CompileShaderFromFile(const WCHAR * const pFileName, const LPCSTR pEntryPoint, const LPCSTR pShaderModel, ID3DBlob ** const pBlobOut);
	HRESULT InitDevice(const HWND& pHWnd);
	HRESULT CreateConstantBuffers();
	HRESULT LoadGeometryBuffers(const Shape& pShape);
	HRESULT LoadTextures(const Shape& pShape);
	HRESULT LoadShaders(const Shape& pShape);
	HRESULT LoadInstanceBuffers(const Shape& pShape);

public:

	explicit DirectXManager(const HWND& pHWnd, AntTweakManager& pAwManager);
	~DirectXManager();
	void Cleanup();

	DirectXManager& operator=(const DirectXManager& pDirectXManager) = delete;
	DirectXManager(const DirectXManager& pDirectXManager) = delete;

	HRESULT Render(const std::vector<GameObject>& pGameObjects, const Camera * const pCam, const std::vector<Light> & pLights, const float pTime);
};

