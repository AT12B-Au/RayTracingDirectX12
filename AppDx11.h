#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <string>

#pragma warning(push)
#pragma warning(disable:4005)

#include <d3d11.h>
#include <d3dx9.h>
#include <d3dx11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "imgui/imgui_impl_dx11.h"

#pragma warning(pop)

#pragma comment (lib,"winmm.lib")
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dx9.lib")
#pragma comment (lib,"d3dx11.lib")
#pragma comment (lib,"dxgi.lib")

//using namespace DirectX;
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

#define GetDX11Renderer	RendererManager::m_mainDx11

struct MATERIAL11
{
	D3DXCOLOR	Ambient;
	D3DXCOLOR	Diffuse;
	D3DXCOLOR	Specular;
	D3DXCOLOR	Emission;
	float		Shininess;
	float		Dummy[3];
};

struct GlobalBuffer11
{
	D3DXVECTOR4 g_CameraPosition;//視点位置
	D3DXVECTOR4 g_Parameter;
	MATERIAL11 material;
};

struct VERTEX_3D11
{
	D3DXVECTOR3 Position;
	D3DXVECTOR3 Normal;
	D3DXVECTOR4 Diffuse;
	D3DXVECTOR2 TexCoord;
};

struct LIGHT11
{
	BOOL		Enable;
	BOOL		Dummy[3];
	D3DXVECTOR4	Direction;
	D3DXCOLOR	Diffuse;
	D3DXCOLOR	Ambient;
	D3DXMATRIX	ViewMatrix;			//ライトカメラ行列
	D3DXMATRIX	ProjectionMatrix;	//ライトプロジェクション行列
};

class AppDx11
{
private:

	D3D_FEATURE_LEVEL       m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	ComPtr<ID3D11Device> m_Device = NULL;
	ComPtr<ID3D11DeviceContext> m_DeviceContext = NULL;
	ComPtr<IDXGISwapChain> m_SwapChain = NULL;
	ComPtr<ID3D11RenderTargetView> m_RenderTargetView = NULL;
	ComPtr<ID3D11DepthStencilView> m_DepthStencilView = NULL;

	ComPtr<ID3D11Buffer> m_GlobalBuffer11 = NULL;
	ComPtr<ID3D11Buffer> m_WorldBuffer11 = NULL;
	ComPtr<ID3D11Buffer> m_ViewBuffer11 = NULL;
	ComPtr<ID3D11Buffer> m_ProjectionBuffer11 = NULL;
	//ComPtr<ID3D11Buffer> m_MaterialBuffer11 = NULL;
	ComPtr<ID3D11Buffer> m_LightBuffer11 = NULL;
	//ComPtr<ID3D11Buffer> m_ParameterBuffer11 = NULL;


	ComPtr<ID3D11DepthStencilState> m_DepthStateEnable = NULL;
	ComPtr<ID3D11DepthStencilState> m_DepthStateDisable = NULL;

	static ComPtr<ID3D11DepthStencilView> m_ShadowDepthStencilView;
	static ComPtr<ID3D11ShaderResourceView> m_ShadowDepthShaderResourceView;

public:
	HRESULT Init();
	void UnInit();
	void Begin();
	void End();

	void SetDepthEnable(bool Enable);
	void SetWorldViewProjection2D();
	void SetGlobal(GlobalBuffer11 global);
	void SetWorldMatrix(D3DXMATRIX* WorldMatrix);
	void SetViewMatrix(D3DXMATRIX* ViewMatrix);
	void SetProjectionMatrix(D3DXMATRIX* ProjectionMatrix);
	//void SetMaterial(MATERIAL11 Material);
	void SetLight(LIGHT11 Light);
	//void SetParameter(D3DXVECTOR4 Parameter);

	ComPtr<ID3D11Device> GetDevice() { return m_Device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext() { return m_DeviceContext; }
	ComPtr<ID3D11ShaderResourceView> GetShadowDepthTexture() { return m_ShadowDepthShaderResourceView; }
	void BeginDepth()
	{
		m_DeviceContext->OMSetRenderTargets(0, m_RenderTargetView.GetAddressOf(), m_ShadowDepthStencilView.Get());
		m_DeviceContext->ClearDepthStencilView(m_ShadowDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	void CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName);
	void CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName);
};
