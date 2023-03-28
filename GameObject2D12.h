#pragma once

class GameObject2D12
{
private:
	
	//頂点座標
	VERTEX_3D12* m_Vertex12;

	//頂点バッファ。
	ComPtr<ID3D12Resource> m_VertexBuffer12;		
	ComPtr<ID3D12Resource> m_VertexBufferUpLoad12;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView12;

	//インデックスバッファ
	ComPtr<ID3D12Resource> m_IndexBuffer12;
	ComPtr<ID3D12Resource> m_IndexBufferUpLoad12;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView12;

	//テクスチャ
	ComPtr<ID3D12Resource> m_Texture12;
	ComPtr<ID3D12Resource> m_TextureUpLoad12;
	DXGI_FORMAT m_TextureFormat;
	bool HaveTexture;

	//パイプラインステート
	//ComPtr<ID3D12PipelineState> m_PipelineState;

	ComPtr<ID3D12DescriptorHeap> m_DescHeap;

	ComPtr<ID3D12Resource> m_GlobalBuffer12;
	ComPtr<ID3D12Resource> m_WorldBuffer12;
	ComPtr<ID3D12Resource> m_ViewBuffer12;
	ComPtr<ID3D12Resource> m_ProjectionBuffer12;

	D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_WBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_VBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_PBVDesc12;

	//class RootSignature* m_RootSignature;

public:

	
	void Init(VERTEX_3D12* Vertex12, const wchar_t* TextureName,
		DXGI_FORMAT TextureFormat, UINT TextureWidth, UINT TextureHeight, D3D12_STATIC_SAMPLER_DESC* SamplerDesc,
		UINT NumDescriptors);
	void UnInit();
	void Draw();
	void UpdateVertex(VERTEX_3D12* Vertex12);

	ComPtr<ID3D12DescriptorHeap> GetDescHeap() { return m_DescHeap; }
	ComPtr<ID3D12Resource> GetGlobalBuffer() { return m_GlobalBuffer12; }
	ComPtr<ID3D12Resource> GetWorldBuffer() { return m_WorldBuffer12; }
	ComPtr<ID3D12Resource> GetViewBuffer() { return m_ViewBuffer12; }
	ComPtr<ID3D12Resource> GetProjectionBuffer() { return m_ProjectionBuffer12; }
};