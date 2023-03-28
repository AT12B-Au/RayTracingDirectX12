#pragma once

class ShaderTable
{
private:

	ComPtr<ID3D12Resource> m_ShaderTable = nullptr;
	int m_ShaderTableEntrySize = 0;

public:

	void Init(int size);

	void UnInit()
	{
		if (m_ShaderTable != nullptr)
			m_ShaderTable->Release();
	}

	void Update(ComPtr<ID3D12StateObject> PSO, const WCHAR* RayGenName, const WCHAR** MissName,UINT TotalMissShader,
		const WCHAR** HitGroupName, UINT TotalHitGroup);

	ComPtr<ID3D12Resource> GetShaderTable() { return m_ShaderTable; }
	int GetEntrySize() { return m_ShaderTableEntrySize; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return m_ShaderTable->GetGPUVirtualAddress(); }
};