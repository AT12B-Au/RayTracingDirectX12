#include "AppDx12.h"
#include "GameObject.h"
#include "PointLight.h"
#include "RendererManager.h"
#include "main.h"
#include "GameManager.h"

void PointLight::Init(UINT version)
{
	if (version == 0)
	{

	}
	else if (version == 1)
	{
		auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_Device = GetDX12Renderer->GetDevice();
		auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
		Scene* scene = GameManager::GetScene();

		//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
		m_CommandAllocator->Reset();
		//コマンドリストをリセットする
		m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

		//モデルロード
		{
			m_AiScene = aiImportFile("Asset\\model\\PointLight.fbx", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
			assert(m_AiScene);
		}

		//頂点バッファとインデックスバッファ
		{
			m_VertexBuffer = new VertexBuffer[m_AiScene->mNumMeshes];
			m_IndexBuffer = new IndexBuffer[m_AiScene->mNumMeshes];

#pragma omp parallel for
			for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
			{
				aiMesh* mesh = m_AiScene->mMeshes[m];

				UINT IndexNum = mesh->mNumFaces * mesh->mFaces[0].mNumIndices;
				UINT* index = new UINT[IndexNum];

				//頂点
				//m_VertexBuffer[m].InitModel(mesh, &StructuredVertex);

				VERTEX_RT_3D12* vertex = new VERTEX_RT_3D12[mesh->mNumVertices];

#pragma omp parallel for
				for (UINT v = 0; v < mesh->mNumVertices; v++)	//頂点
				{
					vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
					vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
					vertex[v].TexCoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
					vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
				}

				m_VertexBuffer[m].Init(mesh->mNumVertices * sizeof(VERTEX_RT_3D12), sizeof(VERTEX_RT_3D12));
				m_VertexBuffer[m].Update(vertex);

				//インデックス
#pragma omp parallel for
				for (UINT f = 0; f < mesh->mNumFaces; f++)
				{
					const aiFace* face = &mesh->mFaces[f];

					assert(face->mNumIndices == 3);

					index[f * 3 + 0] = face->mIndices[0];
					index[f * 3 + 1] = face->mIndices[1];
					index[f * 3 + 2] = face->mIndices[2];

					// インデックス番号の範囲を確認
					assert(index[f * 3 + 0] < mesh->mNumVertices);
					assert(index[f * 3 + 1] < mesh->mNumVertices);
					assert(index[f * 3 + 2] < mesh->mNumVertices);
				}

				m_IndexBuffer[m].Init(IndexNum * sizeof(UINT), sizeof(UINT));
				m_IndexBuffer[m].Update(index);

				delete[] vertex;
				delete[] index;
			}
		}

		//アクセラレーションストラクチャーの作成
		//
		//BLAS
		{
			m_BLASBuffers = new BLASBuffers[m_AiScene->mNumMeshes];

#pragma omp parallel for
			for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
			{

				aiMesh* mesh = m_AiScene->mMeshes[m];
				UINT IndexNum = mesh->mNumFaces * 3;

				D3D12_RAYTRACING_GEOMETRY_DESC GeomDesc = {};
				GeomDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
				GeomDesc.Triangles.VertexBuffer.StartAddress = m_VertexBuffer[m].GetGPUAddress();
				GeomDesc.Triangles.VertexBuffer.StrideInBytes = sizeof(VERTEX_RT_3D12);
				GeomDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
				GeomDesc.Triangles.VertexCount = m_AiScene->mNumMeshes * mesh->mNumVertices;
				GeomDesc.Triangles.IndexBuffer = m_IndexBuffer[m].GetGPUAddress();
				GeomDesc.Triangles.IndexCount = IndexNum;
				GeomDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
				GeomDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
				inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
				inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
				inputs.NumDescs = 1;
				inputs.pGeometryDescs = &GeomDesc;
				inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

				D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
				m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

				m_BLASBuffers[m].Init(info.ScratchDataSizeInBytes, info.ResultDataMaxSizeInBytes);

				D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC AS_Desc = {};
				AS_Desc.Inputs = inputs;
				AS_Desc.DestAccelerationStructureData = m_BLASBuffers[m].GetResultBuffer()->GetGPUVirtualAddress();
				AS_Desc.ScratchAccelerationStructureData = m_BLASBuffers[m].GetScratchBuffer()->GetGPUVirtualAddress();

				m_CommandList->BuildRaytracingAccelerationStructure(&AS_Desc, 0, nullptr);

				//UAVバリア
				D3D12_RESOURCE_BARRIER UavBarrier = {};
				UavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
				UavBarrier.UAV.pResource = m_BLASBuffers[m].GetResultBuffer().Get();
				m_CommandList->ResourceBarrier(1, &UavBarrier);
			}
		}

		//TLAS
		{
			m_InitObjectNum = 1;
#pragma omp parallel for
			for (int i = 0; i < m_InitObjectNum; i++)
			{
				D3D12_RAYTRACING_INSTANCE_DESC* InstanceDesc = new D3D12_RAYTRACING_INSTANCE_DESC[m_AiScene->mNumMeshes];

				XMMATRIX world = XMMatrixIdentity();

#pragma omp parallel for
				for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
				{
					InstanceDesc[m].InstanceID = m + i;								//ヒットしたインスタンス番号
					InstanceDesc[m].InstanceContributionToHitGroupIndex = POINT_LIGHT_HIT_GROUP;		//ヒットグループnum
					InstanceDesc[m].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

					XMFLOAT3X4 Store3X4;

					XMStoreFloat3x4(&Store3X4, world);
					ConvertTransform(InstanceDesc[m], Store3X4);

					//BLASとアタッチ
					InstanceDesc[m].AccelerationStructure = m_BLASBuffers[m].GetResultBuffer()->GetGPUVirtualAddress();
					InstanceDesc[m].InstanceMask = 0x08;

					scene->GetInstanceContainer().push_back(InstanceDesc[m]);
				}
			}
		}

		//ライト情報
		{
			m_LightBuffer = new ConstantBuffer;
			m_LightBuffer->Init(sizeof(LightRT), &LightData);

			D3D12_CPU_DESCRIPTOR_HANDLE handle = RayTracingControl::m_DescHeap->GetCPUDescriptorHandleForHeapStart();
			handle.ptr += RayTracingControl::m_DescriptorNum * m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			RayTracingControl::m_DescriptorNum++;
			m_LightBuffer->CreateConstantBufferView(handle);
		}

		//DescriptorTable
		{
			DescriptorTable table[1];
			table[0] = { D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, RayTracingControl::BaseShaderRegisterCBV++, 0,
				D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE, RayTracingControl::TotalTableNum };
			RayTracingControl::TotalTableNum++;
#pragma omp parallel for
			for (int i = 0; i < sizeof(table) / sizeof(table[0]); i++)
				RayTracingControl::m_DescriptorTableData.push_back(table[i]);
		}
	}
}

void PointLight::UnInit(UINT version)
{
	if (version == 0)
	{

	}
	else if (version == 1)
	{
#pragma omp parallel for
		for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
		{
			m_VertexBuffer[m].UnInit();
			m_IndexBuffer[m].UnInit();
			m_BLASBuffers[m].UnInit();
		}

		aiReleaseImport(m_AiScene);
	}
}

void PointLight::Update(UINT version)
{
	if (version == 0)
	{

	}
	else if (version == 1)
	{
		Scene* scene = GameManager::GetScene();
		XMFLOAT3 WorldPos = XMFLOAT3(0.0f,0.0f,0.0f);
		//Update TLAS
		{
#pragma omp parallel for
			for (int i = 0; i < m_InitObjectNum; i++)
			{
				D3D12_RAYTRACING_INSTANCE_DESC* InstanceDesc = new D3D12_RAYTRACING_INSTANCE_DESC[m_AiScene->mNumMeshes];

				m_Position12.y = 4.0f;

				XMFLOAT3 CenterRotationPos = XMFLOAT3(m_Position12.x, m_Position12.y, m_Position12.z - 10.0f);
				static float angle = XM_PI / 4.0f;
				angle += 0.01f;
				m_Rotation12.y = angle;

#pragma omp parallel for
				for (UINT m = 0; m < m_AiScene->mNumMeshes; m++)
				{
					InstanceDesc[m].InstanceID = m + i;								//ヒットしたインスタンス番号
					InstanceDesc[m].InstanceContributionToHitGroupIndex = POINT_LIGHT_HIT_GROUP;		//ヒットグループnum
					InstanceDesc[m].Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;

					XMMATRIX world, CenterTrans, rot, InverseTrans, trans;

					CenterTrans = XMMatrixTranslation(-CenterRotationPos.x, -CenterRotationPos.y, -CenterRotationPos.z);
					rot = XMMatrixRotationRollPitchYaw(m_Rotation12.x, m_Rotation12.y, m_Rotation12.z);
					InverseTrans = XMMatrixTranslation(CenterRotationPos.x, CenterRotationPos.y, CenterRotationPos.z);
					trans = XMMatrixTranslation(m_Position12.x, m_Position12.y, m_Position12.z);

					world = CenterTrans * rot * InverseTrans * trans;

					XMVECTOR origin = XMLoadFloat3(&m_Position12);
					XMVECTOR RotationedVector = XMVector3TransformCoord(origin, world);
					
					XMStoreFloat3(&WorldPos, RotationedVector);

					XMFLOAT3X4 Store3X4;

					XMStoreFloat3x4(&Store3X4, world);
					ConvertTransform(InstanceDesc[m], Store3X4);

					//BLASとアタッチ
					InstanceDesc[m].AccelerationStructure = m_BLASBuffers[m].GetResultBuffer()->GetGPUVirtualAddress();
					InstanceDesc[m].InstanceMask = 8;	//0x08試したけどダメ

					scene->GetInstanceContainer().push_back(InstanceDesc[m]);
				}
			}
		}

		//ライト情報更新
		{
			LightData.color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			LightData.direction = XMFLOAT3(-0.5f, 0.5f, -0.2f);
			LightData.PointLightPos = WorldPos;
			LightData.ShadowRayCount = 1;
			LightData.flag = false;		//平行光源か点光源か
			LightData.AmbientColor = XMFLOAT4(0.2f, 0.2f, 0.2f, 0.2f);

			m_LightBuffer->Update(LightData);
		}
	}
}

void PointLight::Draw(UINT version)
{
	if (version == 0)
	{

	}
	else if (version == 1)
	{

	}
}