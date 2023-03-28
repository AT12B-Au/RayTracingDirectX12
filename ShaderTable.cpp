#include "AppDx12.h"
#include "ShaderTable.h"
#include "RendererManager.h"

void ShaderTable::Init(int size)
{
	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();
	m_ShaderTableEntrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	m_ShaderTableEntrySize = AlignTo(D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT, m_ShaderTableEntrySize);

	int ShaderTableSize = m_ShaderTableEntrySize * size;

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = ShaderTableSize;

	auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);

	hr = m_Device->CreateCommittedResource(&HeapProp, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_ShaderTable));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateShaderTableFailed");
	}

	m_ShaderTable->SetName(L"ShaderTable");
}

void ShaderTable::Update(ComPtr<ID3D12StateObject> PSO, const WCHAR* RayGenName, const WCHAR** MissName,UINT TotalMissShader,const WCHAR** HitGroupName,UINT TotalHitGroup)
{
	uint8_t* data;
	m_ShaderTable->Map(0, nullptr, (void**)&data);

	ComPtr<ID3D12StateObjectProperties> PSO_Props;
	PSO->QueryInterface(IID_PPV_ARGS(&PSO_Props));

	//RayGenシェーダー 0
	memcpy(data, PSO_Props->GetShaderIdentifier(RayGenName), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

	//Missシェーダー 1
	for (UINT i = 0; i < TotalMissShader; i++)
	{
		uint8_t* MissEntry = data + m_ShaderTableEntrySize * (i + 1);
		memcpy(MissEntry, PSO_Props->GetShaderIdentifier(MissName[i]), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}

	//ClosestHitシェーダー 2 (HitGroup)
	for (UINT i = 0; i < TotalHitGroup; i++)
	{
		uint8_t* HitEntry = data + m_ShaderTableEntrySize * (i + 1 + TotalMissShader);
		memcpy(HitEntry, PSO_Props->GetShaderIdentifier(HitGroupName[i]), D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
	}

	m_ShaderTable->Unmap(0, nullptr);
}