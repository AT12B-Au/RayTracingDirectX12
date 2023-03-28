#include "AppDx12.h"
#include "VertexBuffer.h"
#include "RendererManager.h"

void VertexBuffer::Init(int size, int stride)
{
	auto m_Device = GetDX12Renderer->GetDevice();
	auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto BufferSize = CD3DX12_RESOURCE_DESC::Buffer(size);
	HRESULT hr;

	//バーテックスバッファ作成
	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_VertexBuffer));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateVertexBufferFailed");
	}

	m_VertexBuffer->SetName(L"VertexBuffer");

	HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//バーテックスバッファ作成(UpLoad)
	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_VertexBufferUpLoad));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateVertexBufferUpLoadFailed");
	}

	m_VertexBufferUpLoad->SetName(L"VertexBufferUpLoad");

	//バーテックスバッファビュー作成
	m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_VertexBufferView.SizeInBytes = size;
	m_VertexBufferView.StrideInBytes = stride;

	//頂点バッファのデータ
	VertexData.RowPitch = size;
	VertexData.SlicePitch = VertexData.RowPitch;
}

void VertexBuffer::Update(VERTEX_RT_3D12* vertex)
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	D3D12_SUBRESOURCE_DATA VertexData = {};
	VertexData.pData = vertex;
	VertexData.RowPitch = sizeof(VERTEX_RT_3D12) * (m_VertexBufferView.SizeInBytes /
		m_VertexBufferView.StrideInBytes);
	VertexData.SlicePitch = VertexData.RowPitch;
	UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer.Get(),
		m_VertexBufferUpLoad.Get(), 0, 0, 1, &VertexData);
	CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	m_CommandList->ResourceBarrier(1, &trans);
}

void VertexBuffer::InitModel(aiMesh* mesh, boost::container::vector<VERTEX_RT_3D12>* StructuredVertex)
{
	VERTEX_RT_3D12* vertex = new VERTEX_RT_3D12[mesh->mNumVertices];

	#pragma omp parallel for
	for (UINT v = 0; v < mesh->mNumVertices; v++)	//頂点
	{
		vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
		vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
		vertex[v].TexCoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
		vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

		if (StructuredVertex != nullptr)
			StructuredVertex->push_back(vertex[v]);
	}

	Init(mesh->mNumVertices * sizeof(VERTEX_RT_3D12), sizeof(VERTEX_RT_3D12));
	Update(vertex);

	delete[] vertex;
}