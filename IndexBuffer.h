#pragma once
#include "assimp/scene.h"

class IndexBuffer
{
private:

	ComPtr<ID3D12Resource> m_IndexBuffer = nullptr;
	ComPtr<ID3D12Resource> m_IndexBufferUpLoad = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView = {};
	int IndexNum = 0;
	int m_StrideInBytes = 0;
	int m_SizeInBytes = 0;

public:

	void Init(int size,int stride);

	void UnInit()
	{
		if (m_IndexBuffer != nullptr)
			m_IndexBuffer->Release();
		if (m_IndexBufferUpLoad != nullptr)
			m_IndexBufferUpLoad->Release();
	}

	void Update(UINT* index);

	ComPtr<ID3D12Resource> GetIndexBuffer() const { return m_IndexBuffer; }
	ComPtr<ID3D12Resource> GetIndexBufferUpLoad() const { return m_IndexBufferUpLoad; }
	const D3D12_INDEX_BUFFER_VIEW& GetBufferView() const { return m_IndexBufferView; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return m_IndexBuffer->GetGPUVirtualAddress(); }
	int GetIndexNum() { return IndexNum; }
};