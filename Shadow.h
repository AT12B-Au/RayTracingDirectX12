#pragma once

class Shadow
{
private:

	//DirectX11

	//DirectX12
	ID3D12Resource* m_VertexBuffer12;
	ID3D12Resource* m_VertexBufferUpLoad12;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView12;

	ID3D12Resource* m_IndexBuffer12;
	ID3D12Resource* m_IndexBufferUpLoad12;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView12;

	ComPtr<ID3D12DescriptorHeap> m_DescHeap;
	ComPtr<ID3D12Resource> m_constantBuffer12;
	ComPtr<ID3D12Resource> m_WorldBuffer12;
	ComPtr<ID3D12Resource> m_ViewBuffer12;
	ComPtr<ID3D12Resource> m_ProjectionBuffer12;

	D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_wbvDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_vbvDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_pbvDesc12;

	ComPtr<ID3D12RootSignature> m_rootSignature;

	ComPtr<ID3DBlob> m_VertexShader12;
	ComPtr<ID3DBlob> m_PixelShader12;
	ComPtr<ID3D12PipelineState> m_PipelineState12;

public:

	void InitDX11();
	void InitDX12(ID3D12Resource* m_VertexBuffer12, ID3D12Resource* m_vertexBufferUpLoad12, ID3D12Resource* m_indexBuffer12,
				  ID3D12Resource* m_indexBufferUpLoad12, UINT VertexBufferSize);
	void UnInitDX11();
	void UnInitDX12();
	void DrawDX11();
	void DrawDX12();
};