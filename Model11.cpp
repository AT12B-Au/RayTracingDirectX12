#include "Model11.h"

void Model11::Load(const char* FileName)
{
	const std::string modelPath(FileName);
	m_AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(m_AiScene);

	m_VertexBuffer11 = new ID3D11Buffer * [m_AiScene->mNumMeshes];
	m_IndexBuffer11 = new ID3D11Buffer * [m_AiScene->mNumMeshes];

	auto m_Device = GetDX11Renderer->GetDevice().Get();

	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		//頂点バッファ生成
		{
			VERTEX_3D11* vertex = new VERTEX_3D11[mesh->mNumVertices];

			#pragma omp parallel for
			for (UINT v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].Position = D3DXVECTOR3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
				vertex[v].Normal = D3DXVECTOR3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				vertex[v].TexCoord = D3DXVECTOR2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				vertex[v].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_3D11) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex;

			m_Device->CreateBuffer(&bd, &sd, &m_VertexBuffer11[m]);

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

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(UINT) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			m_Device->CreateBuffer(&bd, &sd, &m_IndexBuffer11[m]);

			delete[] index;
		}
	}

	//テクスチャ読み込み
	{
		#pragma omp parallel for
		for (unsigned int m = 0; m < m_AiScene->mNumMaterials; m++)
		{
			aiString path;

			if (m_AiScene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS)
			{
				/*if (path.data[0] == '*')
				{*/
				if (m_Texture11[path.data] == NULL)
				{
					ID3D11ShaderResourceView* texture;
					int id = atoi(&path.data[1]);

					D3DX11CreateShaderResourceViewFromMemory(
						m_Device,
						(const unsigned char*)m_AiScene->mTextures[id]->pcData,
						m_AiScene->mTextures[id]->mWidth,
						NULL, NULL, &texture, NULL);

					m_Texture11[path.data] = texture;
				}
				/*}
				else
				{

				}*/
			}
			else
			{
				m_Texture11[path.data] = NULL;
			}
		}
	}
}

void Model11::InitNormal(const char* TextureName)
{
	D3DX11CreateShaderResourceViewFromFile(GetDX11Renderer->GetDevice().Get(),
		TextureName,
		NULL,
		NULL,
		&m_TextureNormal11,
		NULL);

	assert(m_TextureNormal11);
}

void Model11::UnLoad()
{
	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		m_VertexBuffer11[m]->Release();
		m_IndexBuffer11[m]->Release();
	}

	delete[] m_VertexBuffer11;
	delete[] m_IndexBuffer11;

	//for (std::pair<std::string, ID3D11ShaderResourceView*>pair : m_texture)
	#pragma omp parallel for
	for (auto pair : m_Texture11)
	{
		pair.second->Release();
	}

	aiReleaseImport(m_AiScene);
}

void Model11::Draw()
{
	auto m_DeviceContext = GetDX11Renderer->GetDeviceContext().Get();

	//プリミティブトボロジ設定
	m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		aiMaterial* material = m_AiScene->mMaterials[mesh->mMaterialIndex];

		//テクスチャ設定
		aiString path;
		material->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		m_DeviceContext->PSSetShaderResources(0, 1, &m_Texture11[path.data]);

		if (m_TextureNormal11 != NULL)
			m_DeviceContext->PSSetShaderResources(1, 1, &m_TextureNormal11);

		//頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D11);
		UINT offset = 0;

		m_DeviceContext->IASetVertexBuffers(0, 1, &m_VertexBuffer11[m], &stride, &offset);

		//インデックスバッファ設定
		m_DeviceContext->IASetIndexBuffer(m_IndexBuffer11[m], DXGI_FORMAT_R32_UINT, 0);

		//ポリゴン描画
		m_DeviceContext->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}
