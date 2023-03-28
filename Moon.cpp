#include "GameObject.h"
#include "Model11.h"
#include "Model12.h"
#include "RootSignature.h"
#include "main.h"
#include "Moon.h"
#include "input.h"
#include "camera.h"
#include "GameManager.h"

void Moon::Init(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_Model11 = new Model11;
		m_Model11->Load("Asset\\model\\Moon.fbx");

		GetDX11Renderer->CreateVertexShader(m_VertexShader11.GetAddressOf(), m_VertexLayout11.GetAddressOf(),
			"shader\\normalMappingVS.cso");
		GetDX11Renderer->CreatePixelShader(m_PixelShader11.GetAddressOf(), "shader\\normalMappingPS.cso");

		m_Position11 = D3DXVECTOR3(-1.0f, 4.0f, 3.0f);
		m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Scale11 = D3DXVECTOR3(3.0f, 3.0f, 3.0f);

		m_Model11->InitNormal("Asset\\texture\\MoonNormal.dds");
	}
	else if (version == 1)	//DirectX12
	{
		auto m_Device = GetDX12Renderer->GetDevice();
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
		auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();

		m_Model12 = new Model12(4800, 4800, DDS_FORMAT);
		m_Model12->Load("Asset\\model\\Moon.fbx", NULL,DESCHEAP_NUM + 1);

		//ノーマルマップテクスチャのロード
		{
			//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
			m_CommandAllocator->Reset();
			//コマンドリストをリセットする
			m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

			m_TextureNormal12 = GetDX12Renderer->CreateTexture(DDS_FORMAT, 4800, 4800);
			m_TextureNormalUpLoad12 = GetDX12Renderer->CreateTextureUpLoad(m_TextureNormal12);

			//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
			std::unique_ptr<uint8_t[]> DecodedData;
			std::vector<D3D12_SUBRESOURCE_DATA> Subres;

			HRESULT hr = LoadDDSTextureFromFile(m_Device.Get(), L"Asset\\texture\\MoonNormal.dds", &m_TextureNormal12, DecodedData, Subres);
			if (FAILED(hr))
			{
				throw std::runtime_error("LoadTexture2DFailed");
			}

			GetDX12Renderer->CreateTextureView(m_TextureNormal12, m_Model12->GetDescHeap(), DDS_FORMAT, TEXTURE_LEVEL + 1);

			const UINT SubresSize = static_cast<UINT>(Subres.size());

			//読みだしたテクセルデータを空白テクスチャーに流し込み、テクスチャーとして完成させる
			UpdateSubresources(m_CommandList.Get(), m_TextureNormal12.Get(), m_TextureNormalUpLoad12.Get(), 0, 0, SubresSize, &Subres[0]);
			CD3DX12_RESOURCE_BARRIER tran = CD3DX12_RESOURCE_BARRIER::Transition(m_TextureNormal12.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CommandList->ResourceBarrier(1, &tran);

			//ここで一旦コマンドを閉じる。テクスチャーの転送を開始するため
			m_CommandList->Close();

			//コマンドリストの実行　
			ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
			m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			//同期（待機）　テクスチャーの転送が終わるまで待機
			GetDX12Renderer->WaitGpu();

			m_TextureNormalUpLoad12->Release();
		}
		
		D3D12_STATIC_SAMPLER_DESC sampler;
		//サンプラー
		{
			ZeroMemory(&sampler, sizeof(sampler));
			sampler.Filter = D3D12_FILTER_ANISOTROPIC;
			sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			sampler.MaxAnisotropy = 4;

			//ミップマップ設定
			sampler.MipLODBias = 0;
			sampler.MinLOD = 0.0f;
			sampler.MaxLOD = D3D12_FLOAT32_MAX;
		}

		//ルートシグネチャ
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[DESCHEAP_NUM + 1];
			CD3DX12_ROOT_PARAMETER1 RootParameters[1 + 1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[6].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			//CBV Table
			RootParameters[0].InitAsDescriptorTable(5, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			//SRV Table
			RootParameters[1].InitAsDescriptorTable(2, &ranges[5], D3D12_SHADER_VISIBILITY_ALL);

			m_RootSignature = new RootSignature;
			m_RootSignature->InitCustomize(_countof(RootParameters), RootParameters, &sampler);
		}

		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\normalMappingVS.hlsl");
		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\normalMappingRegi0PS.hlsl");

		m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature());

		//シャドウ
		{
			
		}

		m_Position12 = XMFLOAT3(-1.0f, 4.0f, 3.0f);
		m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Scale12 = XMFLOAT3(3.0f, 3.0f, 3.0f);
	}
}

void Moon::UnInit(UINT version)
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

		m_TextureNormal12->Release();

		m_VertexShader12->Release();
		m_PixelShader12->Release();

		m_PipelineState12->Release();
		m_RootSignature->UnInit();
	}
}

void Moon::Update(UINT version)
{
	if (version == 0)	//DirectX11
	{

	}
	else if (version == 1)	//DirectX12
	{

	}
}

void Moon::Draw(UINT version)
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

			//ライトの情報を構造体へセット
			LIGHT12 light;
			light.Enable = true;
			XMFLOAT4 direct = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
			light.Direction = XMLoadFloat4(&direct);
			light.Direction = XMVector4Normalize(light.Direction);
			light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);//光の当たらない部分の反射係数
			light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//光の当たる部分の反射係数

			GetDX12Renderer->SetLight(&light, m_Model12->GetLightBuffer());
		}

		m_Model12->Draw();
	}
}