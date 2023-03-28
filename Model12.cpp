#include "Model12.h"
#include "main.h"
#include "GameManager.h"
#include "RootSignature.h"
#include "camera.h"

void Model12::Load(const char* FileName, bool shadow,UINT DescHeapNum)
{
	const std::string modelPath(FileName);
	m_AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(m_AiScene);

	m_VertexBuffer12 = new ComPtr<ID3D12Resource>[m_AiScene->mNumMeshes];
	m_VertexBufferUpLoad12 = new ComPtr<ID3D12Resource>[m_AiScene->mNumMeshes];
	m_VertexBufferView12 = new D3D12_VERTEX_BUFFER_VIEW[m_AiScene->mNumMeshes];

	m_IndexBuffer12 = new ComPtr<ID3D12Resource>[m_AiScene->mNumMeshes];
	m_IndexBufferUpLoad12 = new ComPtr<ID3D12Resource>[m_AiScene->mNumMeshes];
	m_IndexBufferView12 = new D3D12_INDEX_BUFFER_VIEW[m_AiScene->mNumMeshes];

	HRESULT hr;
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_commandAllocator = GetDX12Renderer->GetCommandAllocator();
	auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();

	//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
	m_commandAllocator->Reset();
	//コマンドリストをリセットする
	m_CommandList->Reset(m_commandAllocator.Get(), NULL);

	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		//頂点バッファ生成
		{
			VERTEX_3D12* vertex = new VERTEX_3D12[mesh->mNumVertices];

			#pragma omp parallel for
			for (UINT v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
				vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				vertex[v].TexCoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			UINT VertexBufferSize;
			VertexBufferSize = sizeof(VERTEX_3D12) * mesh->mNumVertices;
			auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto desc = CD3DX12_RESOURCE_DESC::Buffer(VertexBufferSize);

			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_VertexBuffer12[m]));

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateVertexBufferFailed");
			}


			prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			//バーテックスバッファ作成(UpLoad)
			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_VertexBufferUpLoad12[m]));

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateVertexBufferUpLoadFailed");
			}

			//バーテックスバッファビュー作成
			m_VertexBufferView12[m].BufferLocation = m_VertexBuffer12[m]->GetGPUVirtualAddress();
			m_VertexBufferView12[m].StrideInBytes = sizeof(VERTEX_3D12);
			m_VertexBufferView12[m].SizeInBytes = VertexBufferSize;



			//頂点バッファの更新
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = &vertex[0];
			vertexData.RowPitch = VertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12[m].Get(), m_VertexBufferUpLoad12[m].Get(), 0, 0, 1, &vertexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12[m].Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);

			delete[] vertex;
		}

		//インデックスバッファ生成
		{
			UINT* index = new UINT[mesh->mNumFaces * 3];

			#pragma omp parallel for
			for (UINT f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				assert(face->mNumIndices == 3);


				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			UINT IndexBufferSize;
			IndexBufferSize = sizeof(UINT) * mesh->mNumFaces * 3;

			auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto desc = CD3DX12_RESOURCE_DESC::Buffer(IndexBufferSize);

			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr,
				IID_PPV_ARGS(&m_IndexBuffer12[m]));
			if (FAILED(hr))
			{
				throw std::runtime_error("CreateIndexBufferFailed");
			}

			prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			hr = m_Device->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(&m_IndexBufferUpLoad12[m]));

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateIndexBufferUpLoadFailed");
			}

			//インデックスバッファビュー作成
			m_IndexBufferView12[m].BufferLocation = m_IndexBuffer12[m]->GetGPUVirtualAddress();
			m_IndexBufferView12[m].Format = DXGI_FORMAT_R32_UINT;
			m_IndexBufferView12[m].SizeInBytes = static_cast<UINT>(IndexBufferSize);

			D3D12_SUBRESOURCE_DATA IndexData = {};
			IndexData.pData = &index[0];
			IndexData.RowPitch = IndexBufferSize;
			IndexData.SlicePitch = IndexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_IndexBuffer12[m].Get(), m_IndexBufferUpLoad12[m].Get(), 0, 0, 1, &IndexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer12[m].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);

			delete[] index;
		}
	}

	//ヒープの作成 && コンスタントバッファの作成
	{
		UINT CBSize;
		CBSize = (sizeof(GlobalBuffer12) + 255) & ~255;

		D3D12_CONSTANT_BUFFER_VIEW_DESC m_CBVDesc12;
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_WBVDesc12;
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_VBVDesc12;
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_PBVDesc12;
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_LBVDesc12;

		m_DescHeap = GetDX12Renderer->CreateConstantBufferViewHeap(DescHeapNum);

		m_GlobalBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_CBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_GlobalBuffer12, m_DescHeap, 0, CBSize);

		CBSize = (sizeof(XMMATRIX) + 255) & ~255;
		m_WorldBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_WBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_WorldBuffer12, m_DescHeap, 1, CBSize);

		m_ViewBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_VBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ViewBuffer12, m_DescHeap, 2, CBSize);

		m_ProjectionBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_PBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ProjectionBuffer12, m_DescHeap, 3, CBSize);

		CBSize = (sizeof(LIGHT12) + 255) & ~255;
		m_LightBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_LBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_LightBuffer12, m_DescHeap, 4, CBSize);
	}

	//テクスチャのロード
	{
		#pragma omp parallel for
		for (unsigned int m = 0; m < m_AiScene->mNumMaterials; m++)
		{
			aiString path;

			if (m_AiScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				/*if (path.data[0] == '*')
				{*/
				if (m_Texture12[path.data] == NULL)
				{

					int id = atoi(&path.data[1]);

					m_Texture12[path.data] = GetDX12Renderer->CreateTexture(TextureFormat, TextureWidth, TextureHeight);
					m_TextureUpLoad12[path.data] = GetDX12Renderer->CreateTextureUpLoad(m_Texture12[path.data]);

					//ファイルからテクセルやサイズ情報を読み出す(この処理は完全にユーザー次第）
					std::unique_ptr<uint8_t[]> decodedData;
					//D3D12_SUBRESOURCE_DATA Subres;
					std::vector<D3D12_SUBRESOURCE_DATA> Subres;

					/*hr = LoadWICTextureFromMemory(m_Device.Get(),
						(const unsigned char*)m_AiScene->mTextures[id]->pcData,
						m_AiScene->mTextures[id]->mWidth, &m_Texture12[path.data], decodedData, Subres);*/
					hr = LoadDDSTextureFromMemory(m_Device.Get(),
						(const unsigned char*)m_AiScene->mTextures[id]->pcData,
						m_AiScene->mTextures[id]->mWidth, &m_Texture12[path.data], Subres);
					if (FAILED(hr))
					{
						throw std::runtime_error("LoadTexture2DFailed");
					}

					//テクスチャビュー
					{
						D3D12_RESOURCE_DESC tdesc;
						ZeroMemory(&tdesc, sizeof(tdesc));
						tdesc.MipLevels = 1;
						tdesc.Format = TextureFormat;
						tdesc.Width = SCREEN_WIDTH;
						tdesc.Height = SCREEN_HEIGHT;
						tdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
						tdesc.DepthOrArraySize = 1;
						tdesc.SampleDesc.Count = 1;
						tdesc.SampleDesc.Quality = 0;
						tdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

						//このテクスチャーのビュー（SRV)を作る
						D3D12_SHADER_RESOURCE_VIEW_DESC sdesc = {};
						sdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
						sdesc.Format = tdesc.Format;
						sdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
						sdesc.Texture2D.MipLevels = 1;
						D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
						/**/handle.ptr += m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * TEXTURE_LEVEL;
						m_Device->CreateShaderResourceView(m_Texture12[path.data].Get(), &sdesc, handle);
					}

					const UINT SubresSize = static_cast<UINT>(Subres.size());

					//読みだしたテクセルデータを空白テクスチャーに流し込み、テクスチャーとして完成させる
					//UpdateSubresources(m_CommandList.Get(), m_Texture12[path.data].Get(), m_TextureUpLoad12[path.data].Get(), 0, 0, 1, &Subres);
					UpdateSubresources(m_CommandList.Get(), m_Texture12[path.data].Get(), m_TextureUpLoad12[path.data].Get(), 0, 0, SubresSize, &Subres[0]);
					CD3DX12_RESOURCE_BARRIER tran = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture12[path.data].Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
					m_CommandList->ResourceBarrier(1, &tran);

					//ここで一旦コマンドを閉じる。テクスチャーの転送を開始するため
					m_CommandList->Close();

					//コマンドリストの実行　
					ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
					m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

					//同期（待機）　テクスチャーの転送が終わるまで待機
					GetDX12Renderer->WaitGpu();

					m_TextureUpLoad12[path.data]->Release();
				}
				/*}
				else
				{

				}*/
			}
			else
			{
				m_Texture12[path.data] = nullptr;
				m_TextureUpLoad12[path.data] = nullptr;
			}
		}
	}

	if (shadow)
	{
		/*m_DescHeapShadow = GetDX12Renderer->CreateConstantBufferViewHeap(4);

		UINT CBSize;
		CBSize = (sizeof(GlobalBuffer12) + 255) & ~255;

		m_GlobalBufferShadow12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_CBVDescShadow12 = GetDX12Renderer->CreateConstantBufferView(m_GlobalBufferShadow12, m_DescHeapShadow, 0, CBSize);

		CBSize = (sizeof(XMMATRIX) + 255) & ~255;
		m_WorldBufferShadow12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_WBVDescShadow12 = GetDX12Renderer->CreateConstantBufferView(m_WorldBufferShadow12, m_DescHeapShadow, 1, CBSize);

		m_ViewBufferShadow12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_VBVDescShadow12 = GetDX12Renderer->CreateConstantBufferView(m_ViewBufferShadow12, m_DescHeapShadow, 2, CBSize);

		m_ProjectionBufferShadow12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
		m_PBVDescShadow12 = GetDX12Renderer->CreateConstantBufferView(m_ProjectionBufferShadow12, m_DescHeapShadow, 3, CBSize);*/
	}
	else
		HaveShadow = false;
}

void Model12::UnLoad()
{
	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		m_VertexBuffer12[m]->Release();
		m_VertexBufferUpLoad12[m]->Release();
		m_IndexBuffer12[m]->Release();
		m_IndexBufferUpLoad12[m]->Release();
	}

	#pragma omp parallel for
	for (auto pair : m_Texture12)
	{
		pair.second->Release();
	}

	aiReleaseImport(m_AiScene);

	m_GlobalBuffer12->Release();
	m_WorldBuffer12->Release();
	m_ViewBuffer12->Release();
	m_ProjectionBuffer12->Release();
	m_LightBuffer12->Release();
	m_DescHeap->Release();

	if (HaveShadow)
	{
		/*m_GlobalBufferShadow12->Release();
		m_WorldBufferShadow12->Release();
		m_ViewBufferShadow12->Release();
		m_ProjectionBufferShadow12->Release();
		m_DescHeapShadow->Release();*/
	}
}

void Model12::Draw()
{
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();

	//ヒープをセット
	ID3D12DescriptorHeap* ppHeaps[] = { m_DescHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	//ポリゴントポロジーの指定
	m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		aiMaterial* material = m_AiScene->mMaterials[mesh->mMaterialIndex];

		//テクスチャ設定
		aiString path;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path);

		//バーテックスバッファをセット
		m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView12[m]);

		//インデックスバッファをセット
		m_CommandList->IASetIndexBuffer(&m_IndexBufferView12[m]);

		//ハンドルセット
		{
			////CBVセット
			D3D12_GPU_DESCRIPTOR_HANDLE CbvSrvUavHandle = m_DescHeap->GetGPUDescriptorHandleForHeapStart();
			m_CommandList->SetGraphicsRootDescriptorTable(0, CbvSrvUavHandle);

			CbvSrvUavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_DescHeap->GetGPUDescriptorHandleForHeapStart(),
				TEXTURE_LEVEL, m_Device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			m_CommandList->SetGraphicsRootDescriptorTable(1, CbvSrvUavHandle);
		}

		m_CommandList->DrawIndexedInstanced(mesh->mNumFaces * 3, 1, 0, 0, 0);

	}
}