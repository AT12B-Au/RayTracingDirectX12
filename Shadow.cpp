#include "AppDx12.h"
#include "RendererManager.h"
#include "Shadow.h"

void Shadow::InitDX11()
{
}

void Shadow::InitDX12(ID3D12Resource* m_vertexBuffer12, ID3D12Resource* m_vertexBufferUpLoad12, ID3D12Resource* m_indexBuffer12,
					  ID3D12Resource* m_indexBufferUpLoad12,UINT VertexBufferSize, VERTEX_3D12* vertex)
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	m_VertexBuffer12 = m_vertexBuffer12;
	m_VertexBufferUpLoad12 = m_vertexBufferUpLoad12;

	m_VertexBufferView12.BufferLocation = m_VertexBuffer12->GetGPUVirtualAddress();
	m_VertexBufferView12.StrideInBytes = sizeof(VERTEX_3D12);
	m_VertexBufferView12.SizeInBytes = VertexBufferSize;

	//頂点バッファの更新
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = &vertex[0];
	vertexData.RowPitch = VertexBufferSize;
	vertexData.SlicePitch = vertexData.RowPitch;
	UpdateSubresources<1>(m_CommandList, m_VertexBuffer12, m_VertexBufferUpLoad12, 0, 0, 1, &vertexData);
	CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12,
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	m_CommandList->ResourceBarrier(1, &trans);

	m_IndexBuffer12 = m_indexBuffer12;
	m_IndexBufferUpLoad12 = m_indexBufferUpLoad12;

	m_IndexBufferView12.BufferLocation = m_indexBuffer12[m]->GetGPUVirtualAddress();
	m_IndexBufferView12.Format = DXGI_FORMAT_R32_UINT;
	m_IndexBufferView12.SizeInBytes = static_cast<UINT>(m_IndexBufferSize);
}

void Shadow::UnInitDX11()
{
}

void Shadow::UnInitDX12()
{
}

void Shadow::DrawDX11()
{
}

void Shadow::DrawDX12()
{
}
