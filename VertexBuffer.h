#pragma once
#include <assimp/mesh.h>

class VertexBuffer
{
private:
	ComPtr<ID3D12Resource> m_VertexBuffer = nullptr;
	ComPtr<ID3D12Resource> m_VertexBufferUpLoad = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView = {};
	D3D12_SUBRESOURCE_DATA VertexData;

public:

	void Init(int size,int stride);

	void UnInit()
	{
		if (m_VertexBuffer != nullptr)
			m_VertexBuffer->Release();
		if (m_VertexBufferUpLoad != nullptr)
			m_VertexBufferUpLoad->Release();
	}

	void Update(VERTEX_RT_3D12* vertex);

	void InitModel(aiMesh* mesh, boost::container::vector<VERTEX_RT_3D12>* StructuredVertex);

	ComPtr<ID3D12Resource> GetVertexBuffer() const { return m_VertexBuffer; }
	ComPtr<ID3D12Resource> GetVertexBufferUpLoad() const { return m_VertexBufferUpLoad; }
	const D3D12_VERTEX_BUFFER_VIEW& GetView() const { return m_VertexBufferView; }
	D3D12_GPU_VIRTUAL_ADDRESS GetGPUAddress() { return m_VertexBuffer->GetGPUVirtualAddress(); }
};