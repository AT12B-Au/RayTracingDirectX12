#include "GameObject.h"
#include "RootSignature.h"
#include "Plane.h"
#include "Model11.h"
#include "Model12.h"
#include "GameManager.h"
#include "camera.h"

Model11* Plane::m_Model11 = NULL;
Model12* Plane::m_Model12 = NULL;
RootSignature* Plane::m_RootSignature = NULL;

void Plane::Init(UINT version)
{
	if (version == 0)	//DirectX11
	{
		GetDX11Renderer->CreateVertexShader(m_VertexShader11.GetAddressOf(), m_VertexLayout11.GetAddressOf(),
			"shader\\DepthShadowMappingVS.cso");
		GetDX11Renderer->CreatePixelShader(m_PixelShader11.GetAddressOf(), "shader\\DepthShadowMappingPS.cso");

		m_Position11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Scale11 = D3DXVECTOR3(0.15f, 0.15f, 0.15f);
	}
	else if (version == 1)	//DirectX12
	{
		//シェーダーの読み込み
		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\PhongVS.hlsl");
		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\PhongPS.hlsl");

		//パイプラインステート
		m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature().Get());

		m_Position12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Scale12 = XMFLOAT3(0.15f, 0.15f, 0.15f);
	}
}

void Plane::UnInit(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_Model11->UnLoad();

		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();
	}
	else if (version == 1)	//DirectX12
	{
		m_RootSignature->UnInit();
		m_PipelineState12->Release();
		m_Model12->UnLoad();
	}
}

void Plane::Update(UINT version)
{
	if (version == 0)	//DirectX11
	{

	}
	else if (version == 1)	//DirectX12
	{

	}
}

void Plane::Draw(UINT version)
{
	if (version == 0)	//DirectX11
	{
		auto m_DeviceContext = GetDX11Renderer->GetDeviceContext().Get();

		//入力レイアウト設定
		m_DeviceContext->IASetInputLayout(m_VertexLayout11.Get());

		//シェーダー設定
		m_DeviceContext->VSSetShader(m_VertexShader11.Get(), NULL, 0);
		m_DeviceContext->PSSetShader(m_PixelShader11.Get(), NULL, 0);

		//ワールドマトリクス設定
		D3DXMATRIX world, scale, rot, trans;
		D3DXMatrixScaling(&scale, m_Scale11.x, m_Scale11.y, m_Scale11.z);
		D3DXMatrixRotationYawPitchRoll(&rot, m_Rotation11.y, m_Rotation11.x, m_Rotation11.z);
		D3DXMatrixTranslation(&trans, m_Position11.x, m_Position11.y, m_Position11.z);

		world = scale * rot * trans;
		GetDX11Renderer->SetWorldMatrix(&world);

		ComPtr<ID3D11ShaderResourceView> ShadowDepthTexture = GetDX11Renderer->GetShadowDepthTexture();
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(1, 1, ShadowDepthTexture.GetAddressOf());

		m_Model11->Draw();
	}
	else if (version == 1)	//DirectX12
	{
		auto m_CommandList = GetDX12Renderer->GetCommandList();

		Scene* scene = GameManager::GetScene();
		Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 1);

		//ルートシグネチャをセット
		m_RootSignature->Draw();

		m_CommandList->SetPipelineState(m_PipelineState12.Get());

		XMMATRIX world, scale, rot, trans;

		scale = XMMatrixScaling(m_Scale12.x, m_Scale12.y, m_Scale12.z);
		rot = XMMatrixRotationRollPitchYaw(m_Rotation12.x, m_Rotation12.y, m_Rotation12.z);
		trans = XMMatrixTranslation(m_Position12.x, m_Position12.y, m_Position12.z);

		world = scale * rot * trans;

		//コンスタントバッファの内容を更新
		{
			MATERIAL12 material;
			material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, alpha);

			GlobalBuffer12 gb;
			gb.g_CameraPosition = XMFLOAT4(camera->GetPosition12().x, camera->GetPosition12().y, camera->GetPosition12().z, 1.0f);
			gb.material = material;

			GetDX12Renderer->SetGlobalBuffer(&gb, m_Model12->GetGlobalBuffer());

			XMMATRIX view, projection;

			view = camera->GetViewMatrix12();
			projection = camera->GetProjMatrix12();

			GetDX12Renderer->SetWorldMatrix(world, m_Model12->GetWorldBuffer());
			GetDX12Renderer->SetViewMatrix(view, m_Model12->GetViewBuffer());
			GetDX12Renderer->SetProjectMatrix(projection, m_Model12->GetProjectionBuffer());
		}

		m_Model12->Draw();
	}
}

void Plane::Load(UINT version)
{
	if (version == 0)
	{
		m_Model11 = new Model11;
		m_Model11->Load("Asset\\model\\Plane1.fbx");
	}
	else if(version == 1)
	{
		//モデルの読み込み
		m_Model12 = new Model12(4096 + 900, 4096 + 900, DDS_FORMAT);
		m_Model12->Load("Asset\\model\\Plane1.fbx", false,DESCHEAP_NUM);

		m_RootSignature = new RootSignature;
		m_RootSignature->InitNormal();
	}
}