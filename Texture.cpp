#include "AppDx12.h"
#include <assimp/scene.h>
#include "Texture.h"
#include "RendererManager.h"
#include "main.h"

void Texture::InitDDSFile(const wchar_t* TexturePath, DXGI_FORMAT TextureFormat, UINT TextureWidth, UINT TextureHeight)
{
	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
	auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();

	//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
	m_CommandAllocator->Reset();
	//コマンドリストをリセットする
	m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

	m_Texture = GetDX12Renderer->CreateTexture(TextureFormat, TextureWidth, TextureHeight);
	m_TextureUpLoad = GetDX12Renderer->CreateTextureUpLoad(m_Texture);
	format = TextureFormat;

	//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
	std::unique_ptr<uint8_t[]> DecodedData;
	std::vector<D3D12_SUBRESOURCE_DATA> Subres;

	hr = LoadDDSTextureFromFile(m_Device.Get(), TexturePath, &m_Texture, DecodedData, Subres);
	if (FAILED(hr))
	{
		throw std::runtime_error("LoadTexture2DFailed");
	}

	const UINT SubresSize = static_cast<UINT>(Subres.size());

	//読みだしたテクセルデータを空白テクスチャーに流し込み、テクスチャーとして完成させる
	UpdateSubresources(m_CommandList.Get(), m_Texture.Get(), m_TextureUpLoad.Get(), 0, 0, SubresSize, &Subres[0]);
	CD3DX12_RESOURCE_BARRIER tran = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	m_CommandList->ResourceBarrier(1, &tran);

	//ここで一旦コマンドを閉じる。テクスチャーの転送を開始するため
	m_CommandList->Close();

	//コマンドリストの実行　
	ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	//同期（待機）　テクスチャーの転送が終わるまで待機
	GetDX12Renderer->WaitGpu();

	m_TextureUpLoad->Release();
}

void Texture::InitDDSMemory(const aiScene* m_AiScene, DXGI_FORMAT TextureFormat, UINT TextureWidth,UINT TextureHeight)
{
	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
	auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
	aiString path;

	if (m_AiScene->mMaterials[0]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
	{
		if (m_Texture == NULL)
		{
			int id = atoi(&path.data[1]);

			m_Texture = GetDX12Renderer->CreateTexture(TextureFormat, TextureWidth, TextureHeight);
			m_TextureUpLoad = GetDX12Renderer->CreateTextureUpLoad(m_Texture);
			format = TextureFormat;

			//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
			std::unique_ptr<uint8_t[]> decodedData;
			std::vector<D3D12_SUBRESOURCE_DATA> Subres;

			hr = LoadDDSTextureFromMemory(m_Device.Get(),
				(const unsigned char*)m_AiScene->mTextures[id]->pcData,
				m_AiScene->mTextures[id]->mWidth, &m_Texture, Subres);
			if (FAILED(hr))
			{
				throw std::runtime_error("LoadTexture2DFailed");
			}

			const UINT SubresSize = static_cast<UINT>(Subres.size());

			//読みだしたテクセルデータを空白テクスチャーに流し込み、テクスチャーとして完成させる
			//UpdateSubresources(m_CommandList.Get(), m_Texture12[path.data].Get(), m_TextureUpLoad12[path.data].Get(), 0, 0, 1, &Subres);
			UpdateSubresources(m_CommandList.Get(), m_Texture.Get(), m_TextureUpLoad.Get(), 0, 0, SubresSize, &Subres[0]);
			CD3DX12_RESOURCE_BARRIER tran = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CommandList->ResourceBarrier(1, &tran);

			//ここで一旦コマンドを閉じる。テクスチャーの転送を開始するため
			m_CommandList->Close();

			//コマンドリストの実行　
			ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
			m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			//同期（待機）　テクスチャーの転送が終わるまで待機
			GetDX12Renderer->WaitGpu();

			m_TextureUpLoad->Release();
		}
	}
	else
	{
		m_Texture = nullptr;
		m_TextureUpLoad = nullptr;
	}
}

void Texture::CreateTextureView(D3D12_CPU_DESCRIPTOR_HANDLE HeapHandle)
{
	auto m_Device = GetDX12Renderer->GetDevice();

	D3D12_RESOURCE_DESC TDesc;
	ZeroMemory(&TDesc, sizeof(TDesc));
	TDesc.MipLevels = 1;
	TDesc.Format = format;
	TDesc.Width = SCREEN_WIDTH;
	TDesc.Height = SCREEN_HEIGHT;
	TDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	TDesc.DepthOrArraySize = 1;
	TDesc.SampleDesc.Count = 1;
	TDesc.SampleDesc.Quality = 0;
	TDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	m_TextureDesc = TDesc;

	//このテクスチャーのビュー（SRV)を作る
	D3D12_SHADER_RESOURCE_VIEW_DESC SDesc = {};
	SDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	SDesc.Format = TDesc.Format;
	SDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	SDesc.Texture2D.MipLevels = 1;

	m_Device->CreateShaderResourceView(m_Texture.Get(), &SDesc, HeapHandle);
}

void Texture::Update()
{
}
