#pragma once

#define WAVE_NUM (60)

class Lake : public GameObject
{
private:

	//DirectX11
	
	VERTEX_3D11 m_Vertex11[WAVE_NUM + 1][WAVE_NUM + 1];

	ID3D11Buffer* m_VertexBuffer11;
	ID3D11Buffer* m_IndexBuffer11;

	ID3D11ShaderResourceView* m_Texture11;
	ID3D11ShaderResourceView* m_TextureNormal11;

	//DirectX12

	VERTEX_3D12 m_Vertex12[WAVE_NUM + 1][WAVE_NUM + 1];

	//頂点バッファ。
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
	ComPtr<ID3D12Resource> m_LightBuffer12;

	//テクスチャ
	ComPtr<ID3D12Resource> m_Texture12;
	ComPtr<ID3D12Resource> m_TextureUpLoad12;

	//ノーマルマップ
	ComPtr<ID3D12Resource> m_TextureNormal12;
	ComPtr<ID3D12Resource> m_TextureNormalUpLoad12;

	class RootSignature* m_RootSignature;
	ComPtr<ID3D12PipelineState> m_PipelineState12;

public:


	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;
	void Draw(UINT version) override;

};