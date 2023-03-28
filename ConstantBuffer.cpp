#include "AppDx12.h"
#include "ConstantBuffer.h"
#include "RendererManager.h"

void ConstantBuffer::Init(int size, void* SrcData)
{
	m_Size = size;

	auto m_Device = GetDX12Renderer->GetDevice();
	HRESULT hr;

	//定数バッファは256バイトアライメントが要求されるので、256の倍数に切り上げる。
	m_AllocSize = (size + 255) & ~255;

	//定数バッファ作成
	int BufferNum = 0;
	auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto BufferSize = CD3DX12_RESOURCE_DESC::Buffer(m_AllocSize);

	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&BufferSize,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_ConstantBuffer));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateConstantBufferFailed");
	}

	m_ConstantBuffer->SetName(L"ConstantBuffer");

	{
		CD3DX12_RANGE readRange(0, 0);        //     intend to read from this resource on the CPU.
		m_ConstantBuffer->Map(0, &readRange, reinterpret_cast<void**>(&m_ConstBufferCPU));
	}

	if (SrcData != nullptr)
		memcpy(m_ConstBufferCPU, SrcData, m_Size);

	m_IsValid = true;
}

void ConstantBuffer::Update(void* data)
{
	UINT BackBufferIndex = GetDX12Renderer->GetBackBufferIndex();
	memcpy(m_ConstBufferCPU, data, m_Size);
}

void ConstantBuffer::CreateConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE DescHandle)
{
	auto m_Device = GetDX12Renderer->GetDevice();
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
	desc.SizeInBytes = m_AllocSize;
	m_Device->CreateConstantBufferView(&desc, DescHandle);
}

D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer::GetGPUVirtualAddress()
{
	return m_ConstantBuffer->GetGPUVirtualAddress();
}