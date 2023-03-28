#include "AppDx12.h"
#include "RootSignature.h"
#include "RendererManager.h"

void RootSignature::InitNormal()
{
	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();

	//サンプラーを作成
	{
		ZeroMemory(&sampler, sizeof(sampler));
		//sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxAnisotropy = 4;
		//sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		//ミップマップ設定
		sampler.MipLODBias = 0;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		/*sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;*/
	}

	CD3DX12_DESCRIPTOR_RANGE1 ranges[DESCHEAP_NUM];
	CD3DX12_ROOT_PARAMETER1 RootParameters[1 + 1];
	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 4, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	ranges[5].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
	//CBV Table
	RootParameters[0].InitAsDescriptorTable(5, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
	//SRV Table
	RootParameters[1].InitAsDescriptorTable(1, &ranges[5], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters,
		1, &sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	hr = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateRootSignatureFailed");
	}
}

void RootSignature::InitCustomize(UINT NumParameters,CD3DX12_ROOT_PARAMETER1* RootParameters, D3D12_STATIC_SAMPLER_DESC* SamplerDesc)
{
	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();

	//サンプラーを作成
	if (SamplerDesc == nullptr)
	{
		ZeroMemory(&sampler, sizeof(sampler));
		//sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxAnisotropy = 4;
		//sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		//ミップマップ設定
		sampler.MipLODBias = 0;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		/*sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;*/
	}
	else
		sampler = *SamplerDesc;

	CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init_1_1(NumParameters, RootParameters,
		1, &sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ID3DBlob* signature;
	ID3DBlob* error;
	hr = D3DX12SerializeVersionedRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (FAILED(hr))
	{
		throw std::runtime_error("D3DX12SerializeVersionedRootSignatureFailed");
		//throw std::runtime_error(static_cast<const char*>(error->GetBufferPointer()));
	}
	hr = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateRootSignatureFailed");
	}
}

void RootSignature::InitGlobalCustomize(UINT NumParameters, D3D12_ROOT_PARAMETER* RootParameters, D3D12_STATIC_SAMPLER_DESC* SamplerDesc)
{
	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();

	//サンプラーを作成
	if (SamplerDesc == NULL)
	{
		ZeroMemory(&sampler, sizeof(sampler));
		//sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.Filter = D3D12_FILTER_ANISOTROPIC;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MaxAnisotropy = 4;
		//sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;

		//ミップマップ設定
		sampler.MipLODBias = 0;
		sampler.MinLOD = 0.0f;
		sampler.MaxLOD = D3D12_FLOAT32_MAX;
		/*sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;*/
	}
	else
		sampler = *SamplerDesc;

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(NumParameters, RootParameters,
		1, &sampler,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
	);

	ComPtr<ID3DBlob> signature;
	ComPtr<ID3DBlob> error;
	hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (FAILED(hr))
	{
		throw std::runtime_error("D3D12SerializeRootSignatureFailed");
		//throw std::runtime_error(static_cast<const char*>(error->GetBufferPointer()));
	}

	hr = m_Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateRootSignatureFailed");
	}
}
void RootSignature::Draw()
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	m_CommandList->SetGraphicsRootSignature(m_RootSignature.Get());
}