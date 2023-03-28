#include "AppDx12.h"
#include "BLASBuffers.h"
#include "RendererManager.h"

void BLASBuffers::Init(uint64_t ScratchSize, uint64_t ResultSize)
{
	auto m_Device = GetDX12Renderer->GetDevice();
	HRESULT hr;

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = ScratchSize;

	auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&m_BLBuffers.scratch));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateScratchBuffer(BLAS)Failed");
	}

	m_BLBuffers.scratch->SetName(L"ScratchBuffer(BLAS)");

	desc.Width = ResultSize;

	hr = m_Device->CreateCommittedResource(
		&HeapProp,
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
		nullptr,
		IID_PPV_ARGS(&m_BLBuffers.result));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateResultBuffer(BLAS)Failed");
	}

	m_BLBuffers.result->SetName(L"ResultBuffer(BLAS)");
}