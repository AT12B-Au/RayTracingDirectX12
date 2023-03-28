#include "GameObject.h"
#include "GameObject2D11.h"
#include "GameObject2D12.h"
#include "RootSignature.h"
#include "Lake.h"
#include "Model11.h"
#include "Model12.h"
#include "main.h"
#include "GameManager.h"
#include "camera.h"
#include "input.h"

#define INDEX_NUM	(((WAVE_NUM + 2) * 2) * WAVE_NUM - 2)
#define WAVE_AMPLITUDE			(10.0f)					// ウェーブポイントの振幅
#define WAVE_LENGTH				(100.0f)				// ウェーブポイントの波長
#define WAVE_CYCLE				(10.0f)					// ウェーブポイントの周期

void Lake::Init(UINT version)
{
	if (version == 0)	//DirectX11
	{
		auto m_Device = GetDX11Renderer->GetDevice();

		//頂点バッファ生成
		{
			//座標
			#pragma omp parallel for
			for (int x = 0; x <= WAVE_NUM; x++)
			{
				#pragma omp parallel for
				for (int z = 0; z <= WAVE_NUM; z++)
				{
					m_Vertex11[x][z].Position = D3DXVECTOR3(-20.0f + z * 0.5f,0.0f,-10.0f + x * 0.25f);
					m_Vertex11[x][z].Normal = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
					m_Vertex11[x][z].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
					m_Vertex11[x][z].TexCoord = D3DXVECTOR2(0.0f + 0.25f * z, 0.0f + 0.25f * x);
				}
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_3D11) * (WAVE_NUM + 1) * (WAVE_NUM + 1);
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = m_Vertex11;

			GetDX11Renderer->GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer11);
		}

		//インデックスバッファ生成
		{
			UINT index[INDEX_NUM];

			int i = 0;

			#pragma omp parallel for
			for (int x = 0; x < WAVE_NUM; x++)
			{
				#pragma omp parallel for
				for (int z = 0; z < (WAVE_NUM + 1); z++)
				{
					index[i] = x * (WAVE_NUM + 1) + z;
					i++;

					index[i] = (x + 1) * (WAVE_NUM + 1) + z;
					i++;
				}

				if (x == WAVE_NUM - 1)
				{
					break;
				}

				index[i] = (x + 1) * (WAVE_NUM + 1) + WAVE_NUM;
				i++;

				index[i] = (x + 1) * (WAVE_NUM + 1);
				i++;
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(UINT) * INDEX_NUM;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			GetDX11Renderer->GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer11);
		}

		m_Position11 = D3DXVECTOR3(0.0f, 0.0f, 5.0f);
		m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Scale11 = D3DXVECTOR3(1.0f, 1.0f, 1.0f);

		//テクスチャ読み込み
		D3DX11CreateShaderResourceViewFromFile(GetDX11Renderer->GetDevice().Get(),
			"Asset/texture/Water 0175.dds",
			NULL,
			NULL,
			&m_Texture11,
			NULL);

		assert(m_Texture11);

		//ノーマルマップ
		D3DX11CreateShaderResourceViewFromFile(GetDX11Renderer->GetDevice().Get(),
			"Asset/texture/Water 0175normal.dds",
			NULL,
			NULL,
			&m_TextureNormal11,
			NULL);

		assert(m_TextureNormal11);

		GetDX11Renderer->CreateVertexShader(&m_VertexShader11, &m_VertexLayout11, "shader\\NormalMap+ShadowVS.cso");
		GetDX11Renderer->CreatePixelShader(&m_PixelShader11, "shader\\NormalMap+ShadowPS.cso");
	}
	else if (version == 1)	//DirectX12
	{
		HRESULT hr;
		auto m_Device = GetDX12Renderer->GetDevice();
		auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();

		//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
		m_CommandAllocator->Reset();
		//コマンドリストをリセットする
		m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

		//頂点バッファ生成
		{
			//座標
			#pragma omp parallel for
			for (int x = 0; x <= WAVE_NUM; x++)
			{
				#pragma omp parallel for
				for (int z = 0; z <= WAVE_NUM; z++)
				{
					m_Vertex12[x][z].Position = XMFLOAT3(-20.0f + z * 0.5f, 0.0f, -10.0f + x * 0.25f);
					m_Vertex12[x][z].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
					m_Vertex12[x][z].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
					m_Vertex12[x][z].TexCoord = XMFLOAT2(0.0f + 0.25f * z, 0.0f + 0.25f * x);
				}
			}

			UINT VertexBufferSize;
			VertexBufferSize = sizeof(VERTEX_3D12) * (WAVE_NUM + 1) * (WAVE_NUM + 1);
			auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto desc = CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize);

			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_VertexBuffer12));

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateVertexBufferFailed");
			}

			prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			//バーテックスバッファ作成(UpLoad)
			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_VertexBufferUpLoad12));

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateVertexBufferUpLoadFailed");
			}

			//バーテックスバッファビュー作成
			m_VertexBufferView12.BufferLocation = m_VertexBuffer12->GetGPUVirtualAddress();
			m_VertexBufferView12.StrideInBytes = sizeof(VERTEX_3D12);
			m_VertexBufferView12.SizeInBytes = VertexBufferSize;

			//頂点バッファの更新
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = &m_Vertex12;
			vertexData.RowPitch = VertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12.Get(), m_VertexBufferUpLoad12.Get(), 0, 0, 1, &vertexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}
		
		//インデックスバッファ生成
		{
			UINT index[INDEX_NUM];

			int i = 0;

			#pragma omp parallel for
			for (int x = 0; x < WAVE_NUM; x++)
			{
				#pragma omp parallel for
				for (int z = 0; z < (WAVE_NUM + 1); z++)
				{
					index[i] = x * (WAVE_NUM + 1) + z;
					i++;

					index[i] = (x + 1) * (WAVE_NUM + 1) + z;
					i++;
				}

				if (x == WAVE_NUM - 1)
				{
					break;
				}

				index[i] = (x + 1) * (WAVE_NUM + 1) + WAVE_NUM;
				i++;

				index[i] = (x + 1) * (WAVE_NUM + 1);
				i++;
			}

			UINT IndexBufferSize;
			IndexBufferSize = sizeof(UINT) * INDEX_NUM;

			auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto desc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);

			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_IndexBuffer12));
			if (FAILED(hr))
			{
				throw std::runtime_error("CreateIndexBufferFailed");
			}

			prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_IndexBufferUpLoad12));

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateIndexBufferUpLoadFailed");
			}

			//インデックスバッファビュー作成
			m_IndexBufferView12.BufferLocation = m_IndexBuffer12->GetGPUVirtualAddress();
			m_IndexBufferView12.Format = DXGI_FORMAT_R32_UINT;
			m_IndexBufferView12.SizeInBytes = static_cast<UINT>(IndexBufferSize);

			D3D12_SUBRESOURCE_DATA IndexData = {};
			IndexData.pData = &index;
			IndexData.RowPitch = IndexBufferSize;
			IndexData.SlicePitch = IndexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_IndexBuffer12.Get(), m_IndexBufferUpLoad12.Get(), 0, 0, 1, &IndexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer12.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//ヒープの作成 && コンスタントバッファの作成
		{
			UINT CBSize;
			CBSize = (sizeof(GlobalBuffer12) + 255) & ~255;

			D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc12;
			D3D12_CONSTANT_BUFFER_VIEW_DESC m_WBVDesc12;
			D3D12_CONSTANT_BUFFER_VIEW_DESC m_VBVDesc12;
			D3D12_CONSTANT_BUFFER_VIEW_DESC m_PBVDesc12;
			D3D12_CONSTANT_BUFFER_VIEW_DESC m_LBVDesc12;

			m_DescHeap = GetDX12Renderer->CreateConstantBufferViewHeap(DESCHEAP_NUM + 1);

			m_GlobalBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_CBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_GlobalBuffer12, m_DescHeap, 0, CBSize);

			CBSize = (sizeof(XMMATRIX) + 255) & ~255;
			m_WorldBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_WBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_WorldBuffer12, m_DescHeap, 1, CBSize);

			m_ViewBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_VBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ViewBuffer12, m_DescHeap, 2, CBSize);

			m_ProjectionBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_PBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ProjectionBuffer12, m_DescHeap, 3, CBSize);

			CBSize = (sizeof(LIGHT12) + 255) & ~255;
			m_LightBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_LBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_LightBuffer12, m_DescHeap, 4, CBSize);

			//SetName
			m_GlobalBuffer12->SetName(L"GlobalBuffer");
			m_WorldBuffer12->SetName(L"WorldBuffer");
			m_ViewBuffer12->SetName(L"ViewBuffer");
			m_ProjectionBuffer12->SetName(L"ProjectionBuffer");
			m_LightBuffer12->SetName(L"LightBuffer");
		}

		//テクスチャのロード
		{
			m_Texture12 = GetDX12Renderer->CreateTexture(DDS_FORMAT, SCREEN_WIDTH, SCREEN_HEIGHT);
			m_TextureUpLoad12 = GetDX12Renderer->CreateTextureUpLoad(m_Texture12);

			//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
			std::unique_ptr<uint8_t[]> DecodedData;
			std::vector<D3D12_SUBRESOURCE_DATA> Subres;

			HRESULT hr = LoadDDSTextureFromFile(m_Device.Get(), L"Asset/texture/Water 0175.dds", &m_Texture12, DecodedData, Subres);
			if (FAILED(hr))
			{
				throw std::runtime_error("LoadTexture2DFailed");
			}

			GetDX12Renderer->CreateTextureView(m_Texture12, m_DescHeap, DDS_FORMAT, TEXTURE_LEVEL);

			const UINT SubresSize = static_cast<UINT>(Subres.size());

			//読みだしたテクセルデータを空白テクスチャーに流し込み、テクスチャーとして完成させる
			UpdateSubresources(m_CommandList.Get(), m_Texture12.Get(), m_TextureUpLoad12.Get(), 0, 0, SubresSize, &Subres[0]);
			CD3DX12_RESOURCE_BARRIER tran = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture12.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CommandList->ResourceBarrier(1, &tran);

			//ここで一旦コマンドを閉じる。テクスチャーの転送を開始するため
			m_CommandList->Close();

			//コマンドリストの実行　
			ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
			m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			//同期（待機）　テクスチャーの転送が終わるまで待機
			GetDX12Renderer->WaitGpu();

			m_TextureUpLoad12->Release();
		}

		//ノーマルマップテクスチャのロード
		{
			//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
			m_CommandAllocator->Reset();
			//コマンドリストをリセットする
			m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

			m_TextureNormal12 = GetDX12Renderer->CreateTexture(DDS_FORMAT, SCREEN_WIDTH, SCREEN_HEIGHT);
			m_TextureNormalUpLoad12 = GetDX12Renderer->CreateTextureUpLoad(m_TextureNormal12);

			//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
			std::unique_ptr<uint8_t[]> DecodedData;
			std::vector<D3D12_SUBRESOURCE_DATA> Subres;

			HRESULT hr = LoadDDSTextureFromFile(m_Device.Get(), L"Asset\\texture\\Water 0175normal.dds", &m_TextureNormal12, DecodedData, Subres);
			if (FAILED(hr))
			{
				throw std::runtime_error("LoadTexture2DFailed");
			}

			GetDX12Renderer->CreateTextureView(m_TextureNormal12.Get(), m_DescHeap, DDS_FORMAT, TEXTURE_LEVEL + 1);

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

		m_Position12 = XMFLOAT3(0.0f, 0.0f, 5.0f);
		m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Scale12 = XMFLOAT3(1.0f, 1.0f, 1.0f);
	}
}

void Lake::UnInit(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_VertexBuffer11->Release();
		m_IndexBuffer11->Release();
		m_Texture11->Release();
		m_TextureNormal11->Release();

		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();
	}
	else if (version == 1)	//DirectX12
	{
		m_VertexBuffer12->Release();
		m_VertexBufferUpLoad12->Release();
		m_IndexBuffer12->Release();
		m_IndexBufferUpLoad12->Release();

		m_GlobalBuffer12->Release();
		m_WorldBuffer12->Release(); 
		m_ViewBuffer12->Release();
		m_ProjectionBuffer12->Release();
		m_LightBuffer12->Release();

		m_Texture12->Release();
		m_TextureNormal12->Release();
		m_DescHeap->Release();

		m_PipelineState12->Release();
		m_RootSignature->UnInit();
	}	
}		

void Lake::Update(UINT version)
{
	if (version == 0)	//DirectX11
	{
		int CenterX, CenterZ;
		float length;
		static float time = 0.0f;

		#pragma omp parallel for
		for (int x = 0; x <= WAVE_NUM; x++)
		{
			#pragma omp parallel for
			for (int z = 0; z <= WAVE_NUM; z++)
			{
				CenterX = CenterZ = (WAVE_NUM + 1) / 2;

				length = m_Vertex11[x][z].Position.x - m_Vertex11[x][0].Position.x;

				m_Vertex11[x][z].Position.y = WAVE_AMPLITUDE * sinf(2.0f * XM_PI * ((length / WAVE_LENGTH) - (time / WAVE_CYCLE))) * 0.004f;

				time += 1.0f;
			}
		}
	}
	else if (version == 1)	//DirectX12
	{
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		int CenterX, CenterZ;
		float length;
		static float time = 0.0f;

		#pragma omp parallel for
		for (int x = 0; x <= WAVE_NUM; x++)
		{
			#pragma omp parallel for
			for (int z = 0; z <= WAVE_NUM; z++)
			{
				CenterX = CenterZ = (WAVE_NUM + 1) / 2;

				length = m_Vertex12[x][z].Position.x - m_Vertex12[x][0].Position.x;

				m_Vertex12[x][z].Position.y = WAVE_AMPLITUDE * sinf(2.0f * XM_PI * ((length / WAVE_LENGTH) - (time / WAVE_CYCLE))) * 0.004f;

				time += 1.0f;
			}
		}
	}
}

void Lake::Draw(UINT version)
{
	if (version == 0)	//DirectX11
	{
		auto m_DeviceContext = GetDX11Renderer->GetDeviceContext();

		//頂点バッファ更新
		{
			//頂点データ書き換え
			D3D11_MAPPED_SUBRESOURCE msr;

			m_DeviceContext->Map(m_VertexBuffer11, 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

			memcpy(msr.pData, &m_Vertex11, sizeof(VERTEX_3D11) * (WAVE_NUM + 1) * (WAVE_NUM + 1));

			m_DeviceContext->Unmap(m_VertexBuffer11, 0);
		}

		//入力レイアウト設定
		m_DeviceContext->IASetInputLayout(m_VertexLayout11.Get());

		//シェーダー設定
		m_DeviceContext->VSSetShader(m_VertexShader11.Get(), NULL, 0);
		m_DeviceContext->PSSetShader(m_PixelShader11.Get(), NULL, 0);

		//マトリクス設定
		D3DXMATRIX world, scale, rot, trans;
		D3DXMatrixScaling(&scale, m_Scale11.x, m_Scale11.y, m_Scale11.z);
		D3DXMatrixRotationYawPitchRoll(&rot, m_Rotation11.y, m_Rotation11.x, m_Rotation11.z);
		D3DXMatrixTranslation(&trans, m_Position11.x, m_Position11.y, m_Position11.z);
		world = scale * rot * trans;
		GetDX11Renderer->SetWorldMatrix(&world);

		//頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D11);
		UINT offset = 0;
		m_DeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer11, &stride, &offset);

		//インデックスバッファ設定
		m_DeviceContext->IASetIndexBuffer(m_IndexBuffer11, DXGI_FORMAT_R32_UINT, 0);

		//マテリアル設定
		MATERIAL11 material;
		ZeroMemory(&material, sizeof(material));
		material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);

		GlobalBuffer11 gb;
		gb.material = material;
		GetDX11Renderer->SetGlobal(gb);

		//テクスチャ設定
		m_DeviceContext->PSSetShaderResources(0, 1, &m_Texture11);
		//ノーマルマップ
		m_DeviceContext->PSSetShaderResources(1, 1, &m_TextureNormal11);

		ComPtr<ID3D11ShaderResourceView> ShadowDepthTexture = GetDX11Renderer->GetShadowDepthTexture();
		m_DeviceContext->PSSetShaderResources(2, 1, ShadowDepthTexture.GetAddressOf());

		//プリミティブポロジ設定
		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//ポリゴン描画
		m_DeviceContext->DrawIndexed(INDEX_NUM, 0, 0);
	}
	else if (version == 1)	//DirectX12
	{
		auto m_Device = GetDX12Renderer->GetDevice();
		auto m_CommandList = GetDX12Renderer->GetCommandList();

		Scene* scene = GameManager::GetScene();
		Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 1);

		//頂点バッファ更新
		{
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = &m_Vertex12[0];
			vertexData.RowPitch = sizeof(VERTEX_3D12) * (WAVE_NUM + 1) * (WAVE_NUM + 1);
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12.Get(), m_VertexBufferUpLoad12.Get(), 0, 0, 1, &vertexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//ルートシグネチャをセット
		m_RootSignature->Draw();

		m_CommandList->SetPipelineState(m_PipelineState12.Get());

		//ヒープをセット
		ID3D12DescriptorHeap* ppHeaps[] = { m_DescHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		//ポリゴントポロジーの指定
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//バーテックスバッファをセット
		m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView12);

		//インデックスバッファをセット
		m_CommandList->IASetIndexBuffer(&m_IndexBufferView12);

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

			GetDX12Renderer->SetGlobalBuffer(&gb, m_GlobalBuffer12);

			XMMATRIX view, projection;

			view = camera->GetViewMatrix12();
			projection = camera->GetProjMatrix12();

			GetDX12Renderer->SetWorldMatrix(world, m_WorldBuffer12);
			GetDX12Renderer->SetViewMatrix(view, m_ViewBuffer12);
			GetDX12Renderer->SetProjectMatrix(projection, m_ProjectionBuffer12);

			//ライトの情報を構造体へセット
			LIGHT12 light;
			light.Enable = true;
			XMFLOAT4 direct = XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
			light.Direction = XMLoadFloat4(&direct);
			light.Direction = XMVector4Normalize(light.Direction);
			light.Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);//光の当たらない部分の反射係数
			light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);//光の当たる部分の反射係数

			GetDX12Renderer->SetLight(&light,m_LightBuffer12);
		}

		//ハンドルセット
		{
			////CBVセット
			D3D12_GPU_DESCRIPTOR_HANDLE CbvSrvUavHandle = m_DescHeap->GetGPUDescriptorHandleForHeapStart();
			m_CommandList->SetGraphicsRootDescriptorTable(0, CbvSrvUavHandle);

			CbvSrvUavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_DescHeap->GetGPUDescriptorHandleForHeapStart(),
				TEXTURE_LEVEL, m_Device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			m_CommandList->SetGraphicsRootDescriptorTable(1, CbvSrvUavHandle);
		}

		m_CommandList->DrawIndexedInstanced(INDEX_NUM, 1, 0, 0, 0);
	}
}