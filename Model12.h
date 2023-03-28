#pragma once
#include "AnimationModel11.h"
#include "AppDx12.h"

class Model12
{
private:
	const aiScene* m_AiScene = NULL;

	//DirectX12

	ComPtr<ID3D12Resource>* m_VertexBuffer12;
	ComPtr<ID3D12Resource>* m_VertexBufferUpLoad12;
	D3D12_VERTEX_BUFFER_VIEW* m_VertexBufferView12;

	ComPtr<ID3D12Resource>* m_IndexBuffer12;
	ComPtr<ID3D12Resource>* m_IndexBufferUpLoad12;
	D3D12_INDEX_BUFFER_VIEW* m_IndexBufferView12;

	ComPtr<ID3D12DescriptorHeap> m_DescHeap;
	ComPtr<ID3D12Resource> m_GlobalBuffer12;
	ComPtr<ID3D12Resource> m_WorldBuffer12;
	ComPtr<ID3D12Resource> m_ViewBuffer12;
	ComPtr<ID3D12Resource> m_ProjectionBuffer12;
	ComPtr<ID3D12Resource> m_LightBuffer12;

	//ComPtr<ID3D12RootSignature> m_RootSignature;
	//class RootSignature* m_RootSignature;

	std::unordered_map<std::string, ComPtr<ID3D12Resource>> m_Texture12;
	std::unordered_map<std::string, ComPtr<ID3D12Resource>> m_TextureUpLoad12;

	ComPtr<ID3DBlob> m_VertexShader12;
	ComPtr<ID3DBlob> m_PixelShader12;
	//ComPtr<ID3D12PipelineState> m_PipelineState12;

	UINT TextureWidth;
	UINT TextureHeight;
	DXGI_FORMAT TextureFormat;

	//‰e—p
	bool HaveShadow;

	/*ComPtr<ID3D12DescriptorHeap> m_DescHeapShadow;
	ComPtr<ID3D12Resource> m_GlobalBufferShadow12;
	ComPtr<ID3D12Resource> m_WorldBufferShadow12;
	ComPtr<ID3D12Resource> m_ViewBufferShadow12;
	ComPtr<ID3D12Resource> m_ProjectionBufferShadow12;

	D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDescShadow12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_WBVDescShadow12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_VBVDescShadow12;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_PBVDescShadow12;*/

public:
	Model12(UINT width, UINT height,DXGI_FORMAT format) {
		TextureWidth = width;
		TextureHeight = height;
		TextureFormat = format;
	};

	~Model12() {};

	void Load(const char* FileName, bool shadow, UINT DescHeapNum);
	void UnLoad();
	void Draw();

	ComPtr<ID3D12DescriptorHeap> GetDescHeap() { return m_DescHeap; }
	//ComPtr<ID3D12DescriptorHeap> GetDescHeapShadow() { return m_DescHeapShadow; }
	ComPtr<ID3D12Resource> GetGlobalBuffer() { return m_GlobalBuffer12; }
	ComPtr<ID3D12Resource> GetWorldBuffer() { return m_WorldBuffer12; }
	ComPtr<ID3D12Resource> GetViewBuffer() { return m_ViewBuffer12; }
	ComPtr<ID3D12Resource> GetProjectionBuffer() { return m_ProjectionBuffer12; }
	ComPtr<ID3D12Resource> GetLightBuffer() { return m_LightBuffer12; }
};