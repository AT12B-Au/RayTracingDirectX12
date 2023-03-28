#pragma once
#include "AnimationModel11.h"
#include <boost/container/vector.hpp>

class AnimationModel12
{
private:
	const aiScene* m_AiScene = NULL;
	std::unordered_map<std::string, const aiScene*> m_Animation;

	//std::vector<DEFORM_VERTEX>* m_DeformVertex;
	boost::container::vector<DEFORM_VERTEX>* m_DeformVertex;		//変形後頂点データ
	//boost::array<DEFORM_VERTEX, 7000>* m_DeformVertex;
	std::unordered_map<std::string, BONE> m_Bone;	//ボーンデータ(名前で参照)

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

	void CreateBone(aiNode* node);
	void UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix);

	//影用
	bool HaveShadow;

public:
	AnimationModel12(UINT width, UINT height, DXGI_FORMAT format)
	{
		TextureWidth = width;
		TextureHeight = height;
		TextureFormat = format;
	}

	void Load(const char* FileName,bool shadow);
	void LoadAnimation(const char* FileName, const char* AnimationName);
	void UnLoad();
	void Update(const char* AnimationName1, const char* AnimationName2, float BlendRate, int frame);
	void Draw();

	ComPtr<ID3D12DescriptorHeap> GetDescHeap() { return m_DescHeap; }
	ComPtr<ID3D12Resource> GetGlobalBuffer() { return m_GlobalBuffer12; }
	ComPtr<ID3D12Resource> GetWorldBuffer() { return m_WorldBuffer12; }
	ComPtr<ID3D12Resource> GetViewBuffer() { return m_ViewBuffer12; }
	ComPtr<ID3D12Resource> GetProjectionBuffer() { return m_ProjectionBuffer12; }
	ComPtr<ID3D12Resource> GetLightBuffer() { return m_LightBuffer12; }
};