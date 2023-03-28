#pragma once

class Texture
{
private:

	ComPtr<ID3D12Resource> m_Texture;
	ComPtr<ID3D12Resource> m_TextureUpLoad;
	D3D12_RESOURCE_DESC m_TextureDesc;
	DXGI_FORMAT format;

public:

	void InitDDSFile(const wchar_t* TexturePath, DXGI_FORMAT TextureFormat, UINT TextureWidth, UINT TextureHeight);
	void InitDDSMemory(const aiScene* m_AiScene, DXGI_FORMAT TextureFormat, UINT TextureWidth, UINT TextureHeight);

	void UnInit()
	{
		if (m_Texture != nullptr)
			m_Texture->Release();
	}

	void CreateTextureView(D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle);
	void Update();
};