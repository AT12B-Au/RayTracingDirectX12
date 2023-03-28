#pragma once

class ConstantBuffer
{
private:

	ComPtr<ID3D12Resource> m_ConstantBuffer = nullptr;
	void* m_ConstBufferCPU = nullptr;	//CPU側からアクセスできるする定数バッファのアドレス。
	int m_Size = 0;
	int m_AllocSize = 0;
	bool m_IsValid = false;

public:

	void Init(int size,void* SrcData = nullptr);

	void UnInit()
	{
		CD3DX12_RANGE readRange(0, 0);

		if (m_ConstantBuffer != nullptr)
		{
			m_ConstantBuffer->Unmap(0, &readRange);
			m_ConstantBuffer->Release();
		}
	}

	bool GetValid() { return m_IsValid; }

	void Update(void* data);
	template< class T>
	void Update(T& data)
	{
		Update(&data);
	}

	void CreateConstantBufferView(D3D12_CPU_DESCRIPTOR_HANDLE DescHandle);

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress();
};