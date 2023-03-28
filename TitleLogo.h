#pragma once

class TitleLogo : public GameObject
{
private:

	//Direct11

	ComPtr<ID3D11Buffer> m_VertexBuffer11;

	ComPtr<ID3D11VertexShader> m_VertexShader11;
	ComPtr<ID3D11PixelShader> m_PixelShader11;
	ComPtr<ID3D11InputLayout> m_VertexLayout11;

	//Direct12

	//頂点バッファ
	ComPtr<ID3D12Resource> m_VertexBuffer12;
	ComPtr<ID3D12Resource> m_VertexBufferUpLoad12;
	D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView12;

	//インデックスバッファ
	ComPtr<ID3D12Resource> m_IndexBuffer12;
	ComPtr<ID3D12Resource> m_IndexBufferUpLoad12;
	D3D12_INDEX_BUFFER_VIEW m_IndexBufferView12;

	ComPtr<ID3D12DescriptorHeap> m_DescHeap;
	ComPtr<ID3D12Resource> m_GlobalBuffer12;
	ComPtr<ID3D12Resource> m_WorldBuffer12;
	ComPtr<ID3D12Resource> m_ViewBuffer12;
	ComPtr<ID3D12Resource> m_ProjectionBuffer12;

	D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_WBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_VBVDesc12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_PBVDesc12;

	//テクスチャ
	ComPtr<ID3D12Resource> m_TextureUpLoad12;

	class RootSignature* m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState12;

public:

	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;
};