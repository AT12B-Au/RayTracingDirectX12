#include <DirectXMath.h>
#include "AppDx12.h"
#include "RendererManager.h"
#include "GameObject2D12.h"
#include "main.h"
#include <boost/container/vector.hpp>

void GameObject2D12::Init(VERTEX_3D12* Vertex12, const wchar_t* TextureName,
	DXGI_FORMAT TextureFormat, UINT TextureWidth, UINT TextureHeight, D3D12_STATIC_SAMPLER_DESC* SamplerDesc,
	UINT NumDescriptors)
{
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
	auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
	//auto m_DescHeap = GetDX12Renderer->GetDescHeap();

	//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
	m_CommandAllocator->Reset();
	//コマンドリストをリセットする
	m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

	m_TextureFormat = TextureFormat;

	//頂点座標
	{
		m_Vertex12 = Vertex12;
	}
	
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

		m_DescHeap = GetDX12Renderer->CreateConstantBufferViewHeap(NumDescriptors);

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

	//テクスチャのロード
	if (TextureName != NULL)
	{
		m_Texture12 = GetDX12Renderer->CreateTexture(m_TextureFormat, TextureWidth, TextureHeight);
		m_TextureUpLoad12 = GetDX12Renderer->CreateTextureUpLoad(m_Texture12);

		//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
		std::unique_ptr<uint8_t[]> DecodedData;
		/*D3D12_SUBRESOURCE_DATA Subres;*/
		std::vector<D3D12_SUBRESOURCE_DATA> Subres;

		/*HRESULT hr = LoadWICTextureFromFile(m_Device.Get(), TextureName, &m_Texture12, DecodedData, Subres);*/

		HRESULT hr = LoadDDSTextureFromFile(m_Device.Get(), TextureName, &m_Texture12, DecodedData, Subres);
		if (FAILED(hr))
		{
			throw std::runtime_error("LoadTexture2DFailed");
		}

		GetDX12Renderer->CreateTextureView(m_Texture12.Get(), m_DescHeap, m_TextureFormat, TEXTURE_LEVEL);

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
	else
		HaveTexture = false;
	////ルートシグネチャ && パイプラインステート
	//{
	//	m_RootSignature = new RootSignature;

	//	//constantBuffer数,texture
	//	CD3DX12_DESCRIPTOR_RANGE1 ranges[DESCHEAP_NUM - 1];
	//	CD3DX12_ROOT_PARAMETER1 RootParameters[1 + 1];
	//	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//	//CBV Table
	//	RootParameters[0].InitAsDescriptorTable(4, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	//	//SRV Table
	//	RootParameters[1].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);

	//	if (SamplerDesc == NULL)
	//		m_RootSignature->InitCustomize(_countof(RootParameters),RootParameters,NULL);
	//	else
	//		m_RootSignature->InitCustomize(_countof(RootParameters), RootParameters,SamplerDesc);

	//	//シェーダー
	//	ComPtr<ID3DBlob> m_VertexShader12;
	//	ComPtr<ID3DBlob> m_PixelShader12;

	//	m_VertexShader12 = GetDX12Renderer->CreateVertexShader(VertexShader12);
	//	m_PixelShader12 = GetDX12Renderer->CreatePixelShader(PixelShader12);

	//	m_PipelineState = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature().Get());
	//}
}

void GameObject2D12::UnInit()
{

	m_WorldBuffer12->Release();
	m_ViewBuffer12->Release();
	m_ProjectionBuffer12->Release();
	m_GlobalBuffer12->Release();
	m_DescHeap->Release();
	if (HaveTexture != false)
		m_Texture12->Release();
	m_VertexBufferUpLoad12->Release();
	m_VertexBuffer12->Release();
	m_IndexBuffer12->Release();
	m_IndexBufferUpLoad12->Release();
	/*m_RootSignature->UnInit();
	m_PipelineState->Release();*/
}

void GameObject2D12::Draw()
{
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	
	////ルートシグネチャをセット
	//m_RootSignature->Draw();
	//
	//m_CommandList->SetPipelineState(m_PipelineState.Get());
	
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

void GameObject2D12::UpdateVertex(VERTEX_3D12* Vertex12)
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	m_Vertex12 = Vertex12;

	//頂点バッファの更新
	const UINT VertexBufferSize = sizeof(VERTEX_3D12) * 4;
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = &m_Vertex12[0];
	vertexData.RowPitch = VertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;
	UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12.Get(), m_VertexBufferUpLoad12.Get(), 0, 0, 1, &vertexData);
	CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_CommandList->ResourceBarrier(1, &trans);
}
