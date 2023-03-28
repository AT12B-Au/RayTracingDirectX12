#include "AppDx11.h"
#include "AppDx12.h"
#include "RendererManager.h"
#include "GameObject.h"
#include "RootSignature.h"
#include "TitleLogo.h"
#include "main.h"
#include "GameObject2D12.h"
#include "input.h"

void TitleLogo::Init(UINT version)
{
	if (version == 0)
	{
		//頂点座標
		VERTEX_3D11 m_Vertex11[4];

		m_Vertex11[0].Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[0].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[0].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[0].TexCoord = D3DXVECTOR2(0.0f, 0.0f);

		m_Vertex11[1].Position = D3DXVECTOR3(600.0f, 0.0f, 0.0f);
		m_Vertex11[1].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[1].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[1].TexCoord = D3DXVECTOR2(1.0f, 0.0f);

		m_Vertex11[2].Position = D3DXVECTOR3(0.0f, 600.0f, 0.0f);
		m_Vertex11[2].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[2].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[2].TexCoord = D3DXVECTOR2(0.0f, 1.0f);

		m_Vertex11[3].Position = D3DXVECTOR3(600.0f, 600.0f, 0.0f);
		m_Vertex11[3].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[3].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[3].TexCoord = D3DXVECTOR2(1.0f, 1.0f);

		// 頂点バッファ生成
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(VERTEX_3D11) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.pSysMem = m_Vertex11;

		GetDX11Renderer->GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer11);

		GetDX11Renderer->CreateVertexShader(&m_VertexShader11, &m_VertexLayout11,
			"shader\\unlitTextureVS.cso");
		GetDX11Renderer->CreatePixelShader(&m_PixelShader11, "shader\\unlitTexturePS.cso");
	}
	else if (version == 1)
	{
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
		auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();

		//頂点座標
		VERTEX_3D12 m_Vertex12[4];

		m_Vertex12[0].Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

		m_Vertex12[1].Position = XMFLOAT3(600.0f, 0.0f, 0.0f);
		m_Vertex12[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

		m_Vertex12[2].Position = XMFLOAT3(0.0f, 600.0f, 0.0f);
		m_Vertex12[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

		m_Vertex12[3].Position = XMFLOAT3(600.0f, 600.0f, 0.0f);
		m_Vertex12[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

		////コマンドリストに書き込む前にはコマンドアロケーターをリセットする
		//m_CommandAllocator->Reset();
		////コマンドリストをリセットする
		//m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

		//バーテックスバッファ作成
		{
			const UINT VertexBufferSize = sizeof(VERTEX_3D12) * 4;
			m_VertexBuffer12 = GetDX12Renderer->CreateVertexBuffer(VertexBufferSize);
			m_VertexBufferUpLoad12 = GetDX12Renderer->CreateVertexBufferUpLoad(VertexBufferSize);
			m_VertexBufferView12 = GetDX12Renderer->CreateVertexBufferView(m_VertexBufferUpLoad12, VertexBufferSize);

			//頂点バッファの更新
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = &m_Vertex12[0];
			vertexData.RowPitch = VertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12.Get(), m_VertexBufferUpLoad12.Get(), 0, 0, 1, &vertexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//インデックスバッファ作成
		{
			DWORD index[6] =
			{
				0,1,2,
				1,3,2
			};

			UINT m_IndexBufferSize = sizeof(index);
			m_IndexBuffer12 = GetDX12Renderer->CreateIndexBuffer(m_IndexBufferSize);
			m_IndexBufferUpLoad12 = GetDX12Renderer->CreateIndexBufferUpLoad(m_IndexBufferSize);
			m_IndexBufferView12 = GetDX12Renderer->CreateIndexBufferView(m_IndexBufferUpLoad12, m_IndexBufferSize);

			D3D12_SUBRESOURCE_DATA IndexData = {};
			IndexData.pData = &index[0];
			IndexData.RowPitch = m_IndexBufferSize;
			IndexData.SlicePitch = IndexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_IndexBuffer12.Get(), m_IndexBufferUpLoad12.Get(), 0, 0, 1, &IndexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer12.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//ヒープの作成 && コンスタントバッファの作成
		{
			UINT CBSize;
			CBSize = (sizeof(GlobalBuffer12) + 255) & ~255;

			m_DescHeap = GetDX12Renderer->CreateConstantBufferViewHeap(5);

			m_GlobalBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);

			m_CBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_GlobalBuffer12, m_DescHeap, 0, CBSize);

			CBSize = (sizeof(XMMATRIX) + 255) & ~255;
			m_WorldBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_WBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_WorldBuffer12, m_DescHeap, 1, CBSize);

			m_ViewBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_VBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ViewBuffer12, m_DescHeap, 2, CBSize);

			m_ProjectionBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_PBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ProjectionBuffer12, m_DescHeap, 3, CBSize);
		}

		//テクスチャロード
		{
			auto m_Texture12 = GetDX12Renderer->GetShadowBuffer();

			m_TextureUpLoad12 = GetDX12Renderer->CreateTextureUpLoad(m_Texture12);

			GetDX12Renderer->CreateShadowView(m_DescHeap, 4);

			D3D12_SUBRESOURCE_DATA TextureData = {};
			TextureData.pData = m_Texture12.Get();
			TextureData.RowPitch = SCREEN_WIDTH * sizeof(UINT); //1行あたりのバイト数
			TextureData.SlicePitch = TextureData.RowPitch * SCREEN_HEIGHT; //1スライスあたりのバイト数

			//読みだしたテクセルデータを空白テクスチャーに流し込み、テクスチャーとして完成させる
			UpdateSubresources(m_CommandList.Get(), m_Texture12.Get(), m_TextureUpLoad12.Get(), 0, 0, 1, &TextureData);
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

		//ルートシグネチャ
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[5];
			CD3DX12_ROOT_PARAMETER1 RootParameters[1 + 1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			//CBV Table
			RootParameters[0].InitAsDescriptorTable(4, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			//SRV Table
			RootParameters[1].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);

			m_RootSignature = new RootSignature;
			m_RootSignature->InitCustomize(_countof(RootParameters), RootParameters, NULL);
		}
		
		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\unlitTextureVS.hlsl");
		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\unlitTexturePS.hlsl");

		m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature());
	}
}

void TitleLogo::UnInit(UINT version)
{
	if (version == 0)
	{
		m_VertexBuffer11->Release();

		//ここにシェーダーオブジェクトの解放を追加
		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();
	}
	else if (version == 1)
	{
		m_VertexBuffer12->Release();
		m_VertexBufferUpLoad12->Release();

		m_IndexBuffer12->Release();
		m_IndexBufferUpLoad12->Release();

		m_GlobalBuffer12->Release();
		m_WorldBuffer12->Release();
		m_ViewBuffer12->Release();
		m_ProjectionBuffer12->Release();

		m_PipelineState12->Release();
		m_RootSignature->UnInit();
	}
}

void TitleLogo::Update(UINT version)
{
	if (version == 0)
	{

	}
	else if (version == 1)
	{
		
	}
}

void TitleLogo::Draw(UINT version)
{
	if (version == 0)
	{
		//ここにシェーダー関連の描画準備を追加
		GetDX11Renderer->GetDeviceContext()->IASetInputLayout(m_VertexLayout11.Get());
		GetDX11Renderer->GetDeviceContext()->VSSetShader(m_VertexShader11.Get(), NULL, 0);
		GetDX11Renderer->GetDeviceContext()->PSSetShader(m_PixelShader11.Get(), NULL, 0);


		// マトリクス設定
		GetDX11Renderer->SetWorldViewProjection2D();


		// 頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D11);
		UINT offset = 0;
		GetDX11Renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, m_VertexBuffer11.GetAddressOf(), &stride, &offset);

		// テクスチャ設定
		ComPtr<ID3D11ShaderResourceView> ShadowDepthTexture = GetDX11Renderer->GetShadowDepthTexture();
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(0, 1, ShadowDepthTexture.GetAddressOf());

		// プリミティブトポロジ設定
		GetDX11Renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		// ポリゴン描画
		GetDX11Renderer->GetDeviceContext()->Draw(4, 0);
	}
	else if (version == 1)
	{
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_Device = GetDX12Renderer->GetDevice();

		//ルートシグネチャをセット
		m_RootSignature->Draw();

		m_CommandList->SetPipelineState(m_PipelineState12.Get());

		//ヒープ（アプリにただ1つだけ）をセット
		ID3D12DescriptorHeap* ppHeaps[] = { m_DescHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		//ポリゴントポロジーの指定
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//バーテックスバッファをセット
		m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView12);

		//インデックスバッファをセット
		m_CommandList->IASetIndexBuffer(&m_IndexBufferView12);

		//ハンドルセット
		{
			//CBVセット
			D3D12_GPU_DESCRIPTOR_HANDLE CbvSrvUavHandle = m_DescHeap->GetGPUDescriptorHandleForHeapStart();
			m_CommandList->SetGraphicsRootDescriptorTable(0, CbvSrvUavHandle);

			CbvSrvUavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_DescHeap->GetGPUDescriptorHandleForHeapStart(),
				TEXTURE_LEVEL, m_Device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			m_CommandList->SetGraphicsRootDescriptorTable(1, CbvSrvUavHandle);
		}

		//描画
		m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
}
