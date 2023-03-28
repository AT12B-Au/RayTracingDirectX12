#pragma once


class RootSignature 
{
private:
	ComPtr<ID3D12RootSignature> m_RootSignature;
	D3D12_STATIC_SAMPLER_DESC sampler;

public:
	void InitNormal();
	void InitCustomize(UINT NumParameters, CD3DX12_ROOT_PARAMETER1* RootParameters, D3D12_STATIC_SAMPLER_DESC* sampler);
	void InitGlobalCustomize(UINT NumParameters, D3D12_ROOT_PARAMETER* RootParameters, D3D12_STATIC_SAMPLER_DESC* SamplerDesc);
	void UnInit()
	{
		if (m_RootSignature != nullptr)
			m_RootSignature->Release();
	}
	void Draw();

	ComPtr<ID3D12RootSignature> GetRootSignature() { return m_RootSignature; }
};