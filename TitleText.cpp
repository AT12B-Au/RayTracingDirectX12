#include "GameObject.h"
#include "RootSignature.h"
#include "Model11.h"
#include "Model12.h"
#include "main.h"
#include "TitleText.h"
#include "input.h"
#include "GameManager.h"
#include "camera.h"

void TitleText::Init(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_Model11 = new Model11;
		m_Model11->Load("Asset\\model\\Title.fbx");

		GetDX11Renderer->CreateVertexShader(m_VertexShader11.GetAddressOf(), m_VertexLayout11.GetAddressOf(),
			"shader\\PhongVS.cso");
		GetDX11Renderer->CreatePixelShader(m_PixelShader11.GetAddressOf(), "shader\\PhongPS.cso");

		m_Position11 = D3DXVECTOR3(-7.0f, 2.0f, 2.0f);
		m_Rotation11 = D3DXVECTOR3(-0.1f, 0.1f, 0.0f);
		m_Scale11 = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
	}
	else if (version == 1)	//DirectX12
	{
		m_Model12 = new Model12(2560, 2560,DDS_FORMAT);
		m_Model12->Load("Asset\\model\\Title.fbx",NULL,DESCHEAP_NUM);

		m_RootSignature = new RootSignature;
		m_RootSignature->InitNormal();

		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\PhongVS.hlsl");
		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\PhongPS.hlsl");

		m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature());

		m_Position12 = XMFLOAT3(-7.0f, 2.0f, 2.0f);
		m_Rotation12 = XMFLOAT3(-0.1f, 0.1f, 0.0f);
		m_Scale12 = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}
}

void TitleText::UnInit(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_Model11->UnLoad();

		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();

		/*m_VertexShaderShadow11->Release();
		m_PixelShaderShadow11->Release();*/
	}
	else if (version == 1)	//DirectX12
	{
		m_Model12->UnLoad();

		m_VertexShader12->Release();
		m_PixelShader12->Release();

		m_PipelineState12->Release();
		m_RootSignature->UnInit();
	}
}

void TitleText::Update(UINT version)
{
	if (version == 0)	//DirectX11
	{
		
	}
	else if (version == 1)	//DirectX12
	{

	}
}

void TitleText::Draw(UINT version)
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

		//マテリアル設定
		MATERIAL11 material;
		ZeroMemory(&material, sizeof(material));
		material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);

		GlobalBuffer11 gb;
		gb.material = material;
		GetDX11Renderer->SetGlobal(gb);

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
			//マテリアル設定
			MATERIAL12 material;
			ZeroMemory(&material, sizeof(material));
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
