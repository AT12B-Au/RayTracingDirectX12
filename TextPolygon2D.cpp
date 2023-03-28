//#include <DirectXMath.h>
//#include "AppDx11.h"
//#include "AppDx12.h"
//#include "RendererManager.h"
//#include "GameObject.h"
//#include "GameObjectDx12.h"
//#include "TextPolygon2D.h"
//#include "main.h"
//#include "GameObject2D11.h"
//#include "GameObject2D12.h"
//
//void TextPolygon2D::Init(UINT version)
//{
//	if (version == 0)	//DirectX11
//	{
//		m_Model2D11 = new GameObject2D11;
//
//		VERTEX_3D11 m_Vertex11[4];
//
//		m_Vertex11[0].Position = D3DXVECTOR3(500.0f, 0.0f, 0.0f);
//		m_Vertex11[0].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
//		m_Vertex11[0].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex11[0].TexCoord = D3DXVECTOR2(0.0f, 0.0f);
//
//		m_Vertex11[1].Position = D3DXVECTOR3(1000.0f, 0.0f, 0.0f);
//		m_Vertex11[1].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
//		m_Vertex11[1].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex11[1].TexCoord = D3DXVECTOR2(1.0f, 0.0f);
//
//		m_Vertex11[2].Position = D3DXVECTOR3(500.0f, 500.0f, 0.0f);
//		m_Vertex11[2].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
//		m_Vertex11[2].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex11[2].TexCoord = D3DXVECTOR2(0.0f, 1.0f);
//
//		m_Vertex11[3].Position = D3DXVECTOR3(1000.0f, 500.0f, 0.0f);
//		m_Vertex11[3].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
//		m_Vertex11[3].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex11[3].TexCoord = D3DXVECTOR2(1.0f, 1.0f);
//
//		m_Model2D11->Init(m_Vertex11, "Asset\\texture\\trace.png", "shader\\PhongVS.cso", "shader\\PhongPS.cso");
//	}
//	else if (version == 1)	//DirectX12
//	{
//		m_Model2D12 = new GameObject2D12;
//
//		m_Vertex12[0].Position = XMFLOAT3(500.0f, 0.0f, 0.0f);
//		m_Vertex12[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_Vertex12[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex12[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
//
//		m_Vertex12[1].Position = XMFLOAT3(1000.0f, 0.0f, 0.0f);
//		m_Vertex12[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_Vertex12[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex12[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
//
//		m_Vertex12[2].Position = XMFLOAT3(500.0f, 500.0f, 0.0f);
//		m_Vertex12[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_Vertex12[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex12[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
//
//		m_Vertex12[3].Position = XMFLOAT3(1000.0f, 500, 0.0f);
//		m_Vertex12[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
//		m_Vertex12[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
//		m_Vertex12[3].TexCoord = XMFLOAT2(1.0f, 1.0f);
//
//		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\PhongVS.hlsl");
//		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\PhongPS.hlsl");
//
//
//
//		m_Model2D12->Init(m_Vertex12, L"Asset\\texture\\trace.png", DXGI_FORMAT_B8G8R8A8_UNORM,
//			SCREEN_WIDTH, SCREEN_HEIGHT, m_VertexShader12.Get(), m_PixelShader12.Get());
//	}
//}
//
//void TextPolygon2D::UnInit(UINT version)
//{
//	if (version == 0)	//DirectX11
//	{
//		m_Model2D11->UnInit();
//	}
//	else if (version == 1)	//DirectX12
//	{
//		m_Model2D12->UnInit();
//	}
//}
//
//void TextPolygon2D::Update(UINT version)
//{
//	if (version == 0)
//	{
//
//	}
//	else if (version == 1)
//	{
//
//	}
//}
//
//void TextPolygon2D::Draw(UINT version)
//{
//	if (version == 0)	//DirectX11
//	{
//		m_Model2D11->Draw();
//	}
//	else if (version == 1)	//DirectX12
//	{
//		m_Model2D12->Draw();
//	}
//}
