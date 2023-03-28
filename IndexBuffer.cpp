#include "AppDx12.h"
#include "IndexBuffer.h"
#include "RendererManager.h"

void IndexBuffer::Init(int size,int stride)
{
	//バイト数設定
	if (stride == 2)
		m_SizeInBytes = size * 2;
	else
		m_SizeInBytes = size;

	m_StrideInBytes = stride;

	auto m_Device = GetDX12Renderer->GetDevice();
	auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto BufferSize = CD3DX12_RESOURCE_DESC::Buffer(m_SizeInBytes);
	HRESULT hr;

	//インデックスバッファ作成
	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_IndexBuffer));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIndexBufferFailed");
	}

	m_IndexBuffer->SetName(L"IndexBuffer");

	//インデックスバッファ作成(UpLoad)
	HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_IndexBufferUpLoad));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIndexBufferUpLoadFailed");
	}

	m_IndexBufferUpLoad->SetName(L"IndexBufferUpLoad");

	//インデックスバッファビュー作成
	m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView.SizeInBytes = static_cast<UINT>(m_SizeInBytes);

	IndexNum = m_SizeInBytes / 4;		//4 = UINT (4バイト数)
}

void IndexBuffer::Update(UINT* index)
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	D3D12_SUBRESOURCE_DATA IndexData = {};
	IndexData.pData = index;
	IndexData.RowPitch = sizeof(UINT) * IndexNum;
	IndexData.SlicePitch = IndexData.RowPitch;
	UpdateSubresources<1>(m_CommandList.Get(), m_IndexBuffer.Get(), m_IndexBufferUpLoad.Get(), 0, 0, 1, &IndexData);
	CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	m_CommandList->ResourceBarrier(1, &trans);
}