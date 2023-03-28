#include "AppDx11.h"
#include "AnimationModel11.h"
#include "main.h"
#include "GameManager.h"
#include "camera.h"


void AnimationModel11::Load(const char* FileName)
{
	const std::string modelPath(FileName);
	m_AiScene = aiImportFile(FileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(m_AiScene);

	m_VertexBuffer11 = new ComPtr<ID3D11Buffer>[m_AiScene->mNumMeshes];
	m_IndexBuffer11 = new ComPtr<ID3D11Buffer>[m_AiScene->mNumMeshes];

	//変形後頂点配列生成
	//m_DeformVertex = new std::vector<DEFORM_VERTEX>[m_AiScene->mNumMeshes];
	m_DeformVertex = new boost::container::vector<DEFORM_VERTEX>[m_AiScene->mNumMeshes];

	//再帰的にボーン生成
	CreateBone(m_AiScene->mRootNode);

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

		//変形後頂点データ	初期化
		#pragma omp parallel for
		for (UINT v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX deformVertex;
			deformVertex.Position = mesh->mVertices[v];
			deformVertex.Normal = mesh->mNormals[v];
			deformVertex.BoneNum = 0;

			#pragma omp parallel for
			for (UINT b = 0; b < 4; b++)
			{
				deformVertex.BoneName[b] = "";
				deformVertex.BoneWeight[b] = 0.0f;
			}

			m_DeformVertex[m].push_back(deformVertex);
		}

		//ボーンデータ初期化
		#pragma omp parallel for
		for (UINT b = 0; b < mesh->mNumBones; b++)
		{
			aiBone* bone = mesh->mBones[b];

			m_Bone[bone->mName.C_Str()].OffsetMatrix = bone->mOffsetMatrix;

			//変形後頂点にボーンデータ格納
			#pragma omp parallel for
			for (UINT w = 0; w < bone->mNumWeights; w++)
			{
				aiVertexWeight weight = bone->mWeights[w];

				int num = m_DeformVertex[m][weight.mVertexId].BoneNum;

				m_DeformVertex[m][weight.mVertexId].BoneWeight[num] = weight.mWeight;
				m_DeformVertex[m][weight.mVertexId].BoneName[num] = bone->mName.C_Str();
				m_DeformVertex[m][weight.mVertexId].BoneNum++;

				assert(m_DeformVertex[m][weight.mVertexId].BoneNum <= 4);
			}
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

void AnimationModel11::LoadAnimation(const char* FileName, const char* AnimationName)
{
	m_Animation[AnimationName] = aiImportFile(FileName, aiProcess_ConvertToLeftHanded);

	assert(m_Animation[AnimationName]);

}

void AnimationModel11::UnLoad()
{
	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		m_VertexBuffer11[m]->Release();
		m_IndexBuffer11[m]->Release();
	}

	delete[] m_DeformVertex;

	//for (std::pair<std::string, ID3D11ShaderResourceView*>pair : m_texture)
	#pragma omp parallel for
	for (auto pair : m_Texture11)
	{
		pair.second->Release();
	}

	aiReleaseImport(m_AiScene);

	#pragma omp parallel for
	for (auto pair : m_Animation)
	{
		aiReleaseImport(pair.second);
	}
}

void AnimationModel11::Update(const char* AnimationName1, const char* AnimationName2,float BlendRate, int frame)
{
	if (!m_Animation[AnimationName1]->HasAnimations())
		return;

	if (!m_Animation[AnimationName2]->HasAnimations())
		return;

	//アニメーションデータからボーンマトリクス算出
	aiAnimation* animation1 = m_Animation[AnimationName1]->mAnimations[0];
	aiAnimation* animation2 = m_Animation[AnimationName2]->mAnimations[0];

	#pragma omp parallel for
	for (UINT c = 0; c < animation1->mNumChannels; c++)
	{
		aiNodeAnim* nodeAnim1 = animation1->mChannels[c];
		aiNodeAnim* nodeAnim2 = animation2->mChannels[c];

		BONE* bone = &m_Bone[nodeAnim1->mNodeName.C_Str()];

		int f;
		f = frame % nodeAnim1->mNumRotationKeys;		//簡易実装
		aiQuaternion rot1 = nodeAnim1->mRotationKeys[f].mValue;

		f = frame % nodeAnim1->mNumPositionKeys;		//簡易実装
		aiVector3D pos1 = nodeAnim1->mPositionKeys[f].mValue;

		f = frame % nodeAnim2->mNumRotationKeys;		//簡易実装
		aiQuaternion rot2 = nodeAnim2->mRotationKeys[f].mValue;

		f = frame % nodeAnim2->mNumPositionKeys;		//簡易実装
		aiVector3D pos2 = nodeAnim2->mPositionKeys[f].mValue;

		aiQuaternion rot;
		aiQuaternion::Interpolate(rot, rot1, rot2, BlendRate);
		
		aiVector3D pos;
		pos = pos1 * (1.0f - BlendRate) + pos2 * BlendRate;

		bone->AnimationMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f), rot, pos);
	}

	//再帰的にボーンマトリクスを更新
	UpdateBoneMatrix(m_AiScene->mRootNode, aiMatrix4x4());

	//頂点変換(CPUスキニング)
	#pragma omp parallel for
	for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		D3D11_MAPPED_SUBRESOURCE ms;
		GetDX11Renderer->GetDeviceContext()->Map(m_VertexBuffer11[m].Get(), 0,
			D3D11_MAP_WRITE_DISCARD, 0, &ms);

		VERTEX_3D11* vertex = (VERTEX_3D11*)ms.pData;

		#pragma omp parallel for
		for (UINT v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX* deformVertex = &m_DeformVertex[m][v];

			aiMatrix4x4 matrix[4];
			aiMatrix4x4 outMatrix;
			matrix[0] = m_Bone[deformVertex->BoneName[0]].Matrix;
			matrix[1] = m_Bone[deformVertex->BoneName[1]].Matrix;
			matrix[2] = m_Bone[deformVertex->BoneName[2]].Matrix;
			matrix[3] = m_Bone[deformVertex->BoneName[3]].Matrix;

			//ウェイトを考慮してマトリクス算出
			//outMatrix = matrix[0] * deformVertex->BoneWeight[0]
			//		    + matrix[1] * deformVertex->BoneWeight[1]
			//		    + matrix[2] * deformVertex->BoneWeight[2]
			//		    + matrix[3] * deformVertex->BoneWeight[3];

			{
				outMatrix.a1 = matrix[0].a1 * deformVertex->BoneWeight[0]
					+ matrix[1].a1 * deformVertex->BoneWeight[1]
					+ matrix[2].a1 * deformVertex->BoneWeight[2]
					+ matrix[3].a1 * deformVertex->BoneWeight[3];

				outMatrix.a2 = matrix[0].a2 * deformVertex->BoneWeight[0]
					+ matrix[1].a2 * deformVertex->BoneWeight[1]
					+ matrix[2].a2 * deformVertex->BoneWeight[2]
					+ matrix[3].a2 * deformVertex->BoneWeight[3];

				outMatrix.a3 = matrix[0].a3 * deformVertex->BoneWeight[0]
					+ matrix[1].a3 * deformVertex->BoneWeight[1]
					+ matrix[2].a3 * deformVertex->BoneWeight[2]
					+ matrix[3].a3 * deformVertex->BoneWeight[3];

				outMatrix.a4 = matrix[0].a4 * deformVertex->BoneWeight[0]
					+ matrix[1].a4 * deformVertex->BoneWeight[1]
					+ matrix[2].a4 * deformVertex->BoneWeight[2]
					+ matrix[3].a4 * deformVertex->BoneWeight[3];



				outMatrix.b1 = matrix[0].b1 * deformVertex->BoneWeight[0]
					+ matrix[1].b1 * deformVertex->BoneWeight[1]
					+ matrix[2].b1 * deformVertex->BoneWeight[2]
					+ matrix[3].b1 * deformVertex->BoneWeight[3];

				outMatrix.b2 = matrix[0].b2 * deformVertex->BoneWeight[0]
					+ matrix[1].b2 * deformVertex->BoneWeight[1]
					+ matrix[2].b2 * deformVertex->BoneWeight[2]
					+ matrix[3].b2 * deformVertex->BoneWeight[3];

				outMatrix.b3 = matrix[0].b3 * deformVertex->BoneWeight[0]
					+ matrix[1].b3 * deformVertex->BoneWeight[1]
					+ matrix[2].b3 * deformVertex->BoneWeight[2]
					+ matrix[3].b3 * deformVertex->BoneWeight[3];

				outMatrix.b4 = matrix[0].b4 * deformVertex->BoneWeight[0]
					+ matrix[1].b4 * deformVertex->BoneWeight[1]
					+ matrix[2].b4 * deformVertex->BoneWeight[2]
					+ matrix[3].b4 * deformVertex->BoneWeight[3];

				outMatrix.c1 = matrix[0].c1 * deformVertex->BoneWeight[0]
					+ matrix[1].c1 * deformVertex->BoneWeight[1]
					+ matrix[2].c1 * deformVertex->BoneWeight[2]
					+ matrix[3].c1 * deformVertex->BoneWeight[3];

				outMatrix.c2 = matrix[0].c2 * deformVertex->BoneWeight[0]
					+ matrix[1].c2 * deformVertex->BoneWeight[1]
					+ matrix[2].c2 * deformVertex->BoneWeight[2]
					+ matrix[3].c2 * deformVertex->BoneWeight[3];

				outMatrix.c3 = matrix[0].c3 * deformVertex->BoneWeight[0]
					+ matrix[1].c3 * deformVertex->BoneWeight[1]
					+ matrix[2].c3 * deformVertex->BoneWeight[2]
					+ matrix[3].c3 * deformVertex->BoneWeight[3];

				outMatrix.c4 = matrix[0].c4 * deformVertex->BoneWeight[0]
					+ matrix[1].c4 * deformVertex->BoneWeight[1]
					+ matrix[2].c4 * deformVertex->BoneWeight[2]
					+ matrix[3].c4 * deformVertex->BoneWeight[3];

				outMatrix.d1 = matrix[0].d1 * deformVertex->BoneWeight[0]
					+ matrix[1].d1 * deformVertex->BoneWeight[1]
					+ matrix[2].d1 * deformVertex->BoneWeight[2]
					+ matrix[3].d1 * deformVertex->BoneWeight[3];

				outMatrix.d2 = matrix[0].d2 * deformVertex->BoneWeight[0]
					+ matrix[1].d2 * deformVertex->BoneWeight[1]
					+ matrix[2].d2 * deformVertex->BoneWeight[2]
					+ matrix[3].d2 * deformVertex->BoneWeight[3];

				outMatrix.d3 = matrix[0].d3 * deformVertex->BoneWeight[0]
					+ matrix[1].d3 * deformVertex->BoneWeight[1]
					+ matrix[2].d3 * deformVertex->BoneWeight[2]
					+ matrix[3].d3 * deformVertex->BoneWeight[3];

				outMatrix.d4 = matrix[0].d4 * deformVertex->BoneWeight[0]
					+ matrix[1].d4 * deformVertex->BoneWeight[1]
					+ matrix[2].d4 * deformVertex->BoneWeight[2]
					+ matrix[3].d4 * deformVertex->BoneWeight[3];
			}

			deformVertex->Position = mesh->mVertices[v];
			deformVertex->Position *= outMatrix;

			//法線変換用に移動成分を削除
			outMatrix.a4 = 0.0f;
			outMatrix.b4 = 0.0f;
			outMatrix.c4 = 0.0f;

			deformVertex->Normal = mesh->mNormals[v];
			deformVertex->Normal *= outMatrix;

			//頂点バッファへ書き込み
			vertex[v].Position.x = deformVertex->Position.x;
			vertex[v].Position.y = deformVertex->Position.y;
			vertex[v].Position.z = deformVertex->Position.z;

			vertex[v].Normal.x = deformVertex->Normal.x;
			vertex[v].Normal.y = deformVertex->Normal.y;
			vertex[v].Normal.z = deformVertex->Normal.z;

			vertex[v].TexCoord.x = mesh->mTextureCoords[0][v].x;
			vertex[v].TexCoord.y = mesh->mTextureCoords[0][v].y;

			vertex[v].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		GetDX11Renderer->GetDeviceContext()->Unmap(m_VertexBuffer11[m].Get(), 0);
	}
}

void AnimationModel11::Draw()
{
	auto m_DeviceContext = GetDX11Renderer->GetDeviceContext().Get();
	Scene* scene = GameManager::GetScene();
	Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 0);

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

		//頂点バッファ設定
		UINT stride = sizeof(VERTEX_3D11);
		UINT offset = 0;

		m_DeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer11[m].GetAddressOf(), &stride, &offset);

		//インデックスバッファ設定
		m_DeviceContext->IASetIndexBuffer(m_IndexBuffer11[m].Get(), DXGI_FORMAT_R32_UINT, 0);

		//ポリゴン描画
		m_DeviceContext->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}
}

void AnimationModel11::CreateBone(aiNode* node)
{
	BONE bone;

	m_Bone[node->mName.C_Str()] = bone;

	#pragma omp parallel for
	for (UINT n = 0; n < node->mNumChildren; n++)
	{
		CreateBone(node->mChildren[n]);
	}
}

void AnimationModel11::UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix)
{
	BONE* bone = &m_Bone[node->mName.C_Str()];

	//マトリクスの乗算順番に注意
	aiMatrix4x4 worldMatrix;

	worldMatrix = matrix;
	worldMatrix *= bone->AnimationMatrix;

	bone->Matrix = worldMatrix;
	bone->Matrix *= bone->OffsetMatrix;

	#pragma omp parallel for
	for (UINT n = 0; n < node->mNumChildren; n++)
	{
		UpdateBoneMatrix(node->mChildren[n], worldMatrix);
	}
}