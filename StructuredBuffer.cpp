#include "AppDx12.h"
#include "StructuredBuffer.h"
#include "RendererManager.h"

void StructuredBuffer::Init(size_t SizeOfElement, size_t NumElement, void* InitData)
{
	m_SizeOfElement = SizeOfElement;
	m_NumElement = NumElement;

	auto m_Device = GetDX12Renderer->GetDevice();

	auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto BufferSize = CD3DX12_RESOURCE_DESC::Buffer((m_SizeOfElement * m_NumElement));
	HRESULT hr;

	//バーテックスバッファ作成
	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_StructuredBuffer));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateStructuredBufferFailed");
	}

	m_StructuredBuffer->SetName(L"StructuredBuffer");

	HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	//バーテックスバッファ作成(UpLoad)
	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_StructuredBufferUpLoad));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateStructuredBufferUpLoadFailed");
	}

	m_StructuredBufferUpLoad->SetName(L"StructuredBufferUpLoad");
}

void StructuredBuffer::Uninit()
{
	if (m_StructuredBuffer != nullptr)
		m_StructuredBuffer->Release();
	if (m_StructuredBufferUpLoad != nullptr)
		m_StructuredBufferUpLoad->Release();
}

void StructuredBuffer::Update(void* UpdateData)
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	D3D12_SUBRESOURCE_DATA VertexData = {};
	VertexData.pData = UpdateData;
	VertexData.RowPitch = (m_SizeOfElement * m_NumElement);
	VertexData.SlicePitch = VertexData.RowPitch;
	UpdateSubresources<1>(m_CommandList.Get(), m_StructuredBuffer.Get(),
		m_StructuredBufferUpLoad.Get(), 0, 0, 1, &VertexData);
	CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_StructuredBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
	m_CommandList->ResourceBarrier(1, &trans);
}

void StructuredBuffer::CreateView(D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle)
{
	auto m_Device = GetDX12Renderer->GetDevice();

	D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc;
	ZeroMemory(&SrvDesc, sizeof(SrvDesc));
	SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SrvDesc.Buffer.FirstElement = 0;
	SrvDesc.Buffer.NumElements = static_cast<UINT>(m_NumElement);
	SrvDesc.Buffer.StructureByteStride = static_cast<UINT>(m_SizeOfElement);
	SrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	m_Device->CreateShaderResourceView(
		m_StructuredBuffer.Get(),
		&SrvDesc,
		DescriptorHandle
	);
}