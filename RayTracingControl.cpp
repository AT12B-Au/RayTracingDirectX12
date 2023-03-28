#include "GameManager.h"
#include "RayTracingControl.h"
#include "main.h"
#include <array>
#include "input.h"

TLASBuffers* RayTracingControl::m_TLASBuffers = nullptr;
ComPtr<ID3D12Resource> RayTracingControl::m_OutputBuffer = nullptr;
ConstantBuffer* RayTracingControl::m_CameraBuffer = nullptr;
CameraRT RayTracingControl::CameraData = {};
ShaderTable* RayTracingControl::m_ShaderTable = nullptr;
ComPtr<ID3D12RootSignature> RayTracingControl::m_RayGenLocal = nullptr;
ComPtr<ID3D12StateObject> RayTracingControl::m_PSO = nullptr;
DxilLibrary** RayTracingControl::m_DxilLibrary = nullptr;
RootSignature* RayTracingControl::m_GlobalRootSignature = {};
boost::container::vector<DescriptorTable> RayTracingControl::m_DescriptorTableData = {};
UINT RayTracingControl::BaseShaderRegisterUAV = 1;
UINT RayTracingControl::BaseShaderRegisterSRV = 1;
UINT RayTracingControl::BaseShaderRegisterCBV = 1;
int RayTracingControl::TotalTableNum = 0;
int* RayTracingControl::TableStart = nullptr;
ComPtr<ID3D12DescriptorHeap> RayTracingControl::m_DescHeap = nullptr;
int RayTracingControl::m_DescriptorNum = 3; //Output(UAV) + TLAS(SRV) + Camera (CBV)
Texture* RayTracingControl::m_BackgroundTexture = nullptr;

void RayTracingControl::InitDescHeap()
{
	m_DescHeap = GetDX12Renderer->CreateConstantBufferViewHeap(AllocateDescriptorsNum);
}

void RayTracingControl::Init()
{
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
	auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
	Scene* scene = GameManager::GetScene();

	HRESULT hr;

	//TLAS
	{
		int TotalObjectNum = 0;
#pragma omp parallel for
		for (auto object : scene->GetRTObjectList())
		{
			TotalObjectNum += object->GetInitObjectNum();
		}
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE;
		inputs.NumDescs = TotalObjectNum;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		m_TLASBuffers = new TLASBuffers;
		m_TLASBuffers->Init(info.ScratchDataSizeInBytes, info.ResultDataMaxSizeInBytes, TotalObjectNum);

		D3D12_RAYTRACING_INSTANCE_DESC* InstanceDesc;
		m_TLASBuffers->GetInstanceDescBuffer()->Map(0, nullptr, (void**)&InstanceDesc);

#pragma omp parallel for
		for (int i = 0;i < TotalObjectNum;i++)
		{
			static int id = 0;

			InstanceDesc[id] = scene->GetInstanceContainer()[id];

			id++;
		}

		m_TLASBuffers->GetInstanceDescBuffer()->Unmap(0, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC AS_Desc = {};
		AS_Desc.Inputs = inputs;
		AS_Desc.Inputs.InstanceDescs = m_TLASBuffers->GetInstanceDescBuffer()->GetGPUVirtualAddress();
		AS_Desc.DestAccelerationStructureData = m_TLASBuffers->GetResultBuffer()->GetGPUVirtualAddress();
		AS_Desc.ScratchAccelerationStructureData = m_TLASBuffers->GetScratchBuffer()->GetGPUVirtualAddress();

		m_CommandList->BuildRaytracingAccelerationStructure(&AS_Desc, 0, nullptr);

		//UAVバリア
		D3D12_RESOURCE_BARRIER UavBarrier = {};
		UavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
		UavBarrier.UAV.pResource = m_TLASBuffers->GetResultBuffer().Get();
		m_CommandList->ResourceBarrier(1, &UavBarrier);

		scene->GetInstanceContainer().clear();
	}

	m_CommandList->Close();
	ComPtr<ID3D12CommandList> ppCommandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists->GetAddressOf());

	GetDX12Renderer->WaitGpu();

	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
	m_CommandList->Close();

	const UINT TotalMissShader = sizeof(MissShaderName) / sizeof(*MissShaderName);
	const UINT TotalHitShader = sizeof(ClosestHitShaderName) / sizeof(*ClosestHitShaderName);

	int EntriesNum = 0;
	/// PSOの作成
	{
#ifdef _DEBUG
		std::array<D3D12_STATE_SUBOBJECT, (6 + TotalMissShader + TotalHitShader * 3)> SubObjectArray = {};
#else
		boost::container::vector<D3D12_STATE_SUBOBJECT> SubObjectArray;
#endif
		//DXILライブラリの作成(シェーダーとそのエントリーポイントを含む）
		D3D12_STATE_SUBOBJECT SubObject = {};
		UINT index = 0;
		boost::container::vector<const WCHAR*> EntryPoints;
		{
			m_DxilLibrary = new DxilLibrary * [TotalHitShader];

#pragma omp parallel for
			for (int i = 0; i < TotalHitShader; i++)
			{
				m_DxilLibrary[i] = new DxilLibrary;
				m_DxilLibrary[i]->Init(ShaderName[i], L"lib_6_3");
				if (i == 0)
				{
					EntryPoints.push_back(RayGenShaderName);	//RayGen
					EntriesNum++;
#pragma omp parallel for
					for (int j = 0; j < TotalMissShader; j++)
					{
						EntryPoints.push_back(MissShaderName[j]);		//Miss
						EntriesNum++;
					}
					EntryPoints.push_back(ClosestHitShaderName[i]);		//Hit
					EntriesNum++;
				}
				else
				{
#pragma omp parallel for
					for (int j = 0; j < TotalMissShader; j++)
					{
						EntryPoints.push_back(MissShaderName[j]);		//Miss
						EntriesNum++;
					}
					EntryPoints.push_back(ClosestHitShaderName[i]);		//Hit
					EntriesNum++;
				}
				if (i == 0)
					SubObject = m_DxilLibrary[i]->CreateDxilLibrary(EntryPoints.data(), 2 + TotalMissShader);
				else
					SubObject = m_DxilLibrary[i]->CreateDxilLibrary(&ClosestHitShaderName[i], 1);
#ifdef _DEBUG
				SubObjectArray[index] = SubObject;
#else
				SubObjectArray.push_back(SubObject);
#endif
				index++;
			}
		}
		
		//HitGroup作成
		{
#pragma omp parallel for
			for (int i = 0; i < TotalHitShader; i++)
			{
				D3D12_HIT_GROUP_DESC* HitGroupDesc = new D3D12_HIT_GROUP_DESC;

				ZeroMemory(&SubObject, sizeof(D3D12_STATE_SUBOBJECT));
				ZeroMemory(HitGroupDesc, sizeof(D3D12_HIT_GROUP_DESC));
				HitGroupDesc->AnyHitShaderImport = nullptr;
				HitGroupDesc->ClosestHitShaderImport = ClosestHitShaderName[i];
				HitGroupDesc->HitGroupExport = HitGroupName[i];

				SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
				SubObject.pDesc = HitGroupDesc;

#ifdef _DEBUG
				SubObjectArray[index] = SubObject;
#else
				SubObjectArray.push_back(SubObject);
#endif
				index++;
			}
		}

		//RayGen用のローカルルートシグネチャの作成
		{
			D3D12_DESCRIPTOR_RANGE ranges[3] = {};

			// gOutput
			ranges[0].BaseShaderRegister = 0;
			ranges[0].NumDescriptors = 1;
			ranges[0].RegisterSpace = 0;
			ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
			ranges[0].OffsetInDescriptorsFromTableStart = 0;

			// gRtScene
			ranges[1].BaseShaderRegister = 0;
			ranges[1].NumDescriptors = 1;
			ranges[1].RegisterSpace = 0;
			ranges[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			ranges[1].OffsetInDescriptorsFromTableStart = 1;

			//CBV
			ranges[2].BaseShaderRegister = 0;
			ranges[2].NumDescriptors = 1;
			ranges[2].RegisterSpace = 0;
			ranges[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
			ranges[2].OffsetInDescriptorsFromTableStart = 2;

			D3D12_ROOT_PARAMETER rootParams = {};
			rootParams.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			rootParams.DescriptorTable.NumDescriptorRanges = 3;			//テーブルの中に3個しかない
			rootParams.DescriptorTable.pDescriptorRanges = &ranges[0];

			D3D12_ROOT_SIGNATURE_DESC RootSignDesc = {};
			RootSignDesc.NumParameters = 1;		//テーブル1個のみ
			RootSignDesc.pParameters = &rootParams;
			RootSignDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;

			ComPtr<ID3DBlob> SignBlob;
			ComPtr<ID3DBlob> ErrorBlob;

			hr = D3D12SerializeRootSignature(&RootSignDesc, D3D_ROOT_SIGNATURE_VERSION_1, &SignBlob, &ErrorBlob);
			if (FAILED(hr))
			{
				throw std::runtime_error("SerializeLocalRootSignature(RayGen)Failed");
			}
			hr = m_Device->CreateRootSignature(0, SignBlob->GetBufferPointer(), SignBlob->GetBufferSize(), IID_PPV_ARGS(&m_RayGenLocal));
			if (FAILED(hr))
			{
				throw std::runtime_error("CreateLocalRootSignature(RayGen)Failed");
			}

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE;
			SubObject.pDesc = m_RayGenLocal.GetAddressOf();

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;
		}

		int SignNum = index - 1;

		// RayGen用のローカルルートシグネチャのアソシエーションの作成
		{
			ZeroMemory(&SubObject, sizeof(D3D12_STATE_SUBOBJECT));
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* RayGenAssociation = new D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			ZeroMemory(RayGenAssociation, sizeof(D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION));
			RayGenAssociation->NumExports = 1; // RayGenシェーダーだけ
			RayGenAssociation->pExports = &RayGenShaderName;
			
			RayGenAssociation->pSubobjectToAssociate = &SubObjectArray[SignNum]; // ローカルルートシグネチャバン番号

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			SubObject.pDesc = RayGenAssociation;

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;
		}

		//Miss用のローカルルートシグネチャのアソシエーションの作成
		{
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* MissAssociation = new D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			ZeroMemory(&SubObject, sizeof(D3D12_STATE_SUBOBJECT));
			ZeroMemory(MissAssociation, sizeof(D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION));
			MissAssociation->NumExports = TotalMissShader;
			MissAssociation->pExports = MissShaderName;
			MissAssociation->pSubobjectToAssociate = &SubObjectArray[SignNum];

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			SubObject.pDesc = MissAssociation;

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;
		}

		//closesthit用のローカルルートシグネチャのアソシエーションの作成
		{
			for (int i = 0; i < TotalHitShader; i++)
			{
				D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* HitAssociation = new D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				ZeroMemory(&SubObject, sizeof(D3D12_STATE_SUBOBJECT));
				ZeroMemory(HitAssociation, sizeof(D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION));
				HitAssociation->NumExports = 1;
				HitAssociation->pExports = &ClosestHitShaderName[i];
				HitAssociation->pSubobjectToAssociate = &SubObjectArray[SignNum];

				SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
				SubObject.pDesc = HitAssociation;

#ifdef _DEBUG
				SubObjectArray[index] = SubObject;
#else
				SubObjectArray.push_back(SubObject);
#endif
				index++;
			}
		}

		//シェーダーコンフィグの作成
		{
			D3D12_RAYTRACING_SHADER_CONFIG* ShaderConfig = new D3D12_RAYTRACING_SHADER_CONFIG;
			ZeroMemory(ShaderConfig, sizeof(D3D12_RAYTRACING_SHADER_CONFIG));
			ShaderConfig->MaxAttributeSizeInBytes = sizeof(float) * 2;	//重心座標系float2 常に
			ShaderConfig->MaxPayloadSizeInBytes = sizeof(PayLoad);		//Payload

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
			SubObject.pDesc = ShaderConfig;

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;
		}

		int ConfigNum = index - 1;

		// シェーダーコンフィグのアソシエーション
		{
			ZeroMemory(&SubObject, sizeof(D3D12_STATE_SUBOBJECT));
			D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION* ShaderAssociation = new D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			ZeroMemory(ShaderAssociation, sizeof(D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION));
			ShaderAssociation->NumExports = EntriesNum;
			ShaderAssociation->pExports = EntryPoints.data();
			ShaderAssociation->pSubobjectToAssociate = &SubObjectArray[ConfigNum];

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION;
			SubObject.pDesc = ShaderAssociation;

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;
		}

		//パイプラインコンフィグの作成
		{
			D3D12_RAYTRACING_PIPELINE_CONFIG* config = new D3D12_RAYTRACING_PIPELINE_CONFIG;

			config->MaxTraceRecursionDepth = 4;		//1回反射のみ　影は2回以上

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
			SubObject.pDesc = config;

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;
		}


		//グローバルルートシグネチャ
		if (TotalTableNum >= 1)
		{
			int RangesSize = static_cast<int>(m_DescriptorTableData.size());
			CD3DX12_DESCRIPTOR_RANGE1* ranges = new CD3DX12_DESCRIPTOR_RANGE1[RangesSize];
			CD3DX12_ROOT_PARAMETER1* RootParameters = new CD3DX12_ROOT_PARAMETER1[TotalTableNum];
			TableStart = new int[TotalTableNum];

			#pragma omp parallel for
			for (const auto& data : m_DescriptorTableData)
			{
				static int DataNum = 0;

				ranges[DataNum].Init(data.RangeType, data.NumDescriptors, data.BaseShaderRegister,
					data.RegisterSpace, data.flags);
				DataNum++;
			}

			for (int i = 0; i < TotalTableNum; i++)
			{
				int TableSize = 0;
				static int LastTableSize = 3;
				static int RangesStart = 0;
				for (const auto& data : m_DescriptorTableData)
				{
					if (data.TableNum == i)
						TableSize++;
				}

				RootParameters[i].InitAsDescriptorTable(TableSize, &ranges[RangesStart], D3D12_SHADER_VISIBILITY_ALL);
				RangesStart += TableSize;

				if (i == 0)
					TableStart[0] = LastTableSize;//デフォルトUAV + SRV + Camera Offset
				else
					TableStart[i] = TableStart[i - 1] + LastTableSize;

				LastTableSize = TableSize;
			}

			m_GlobalRootSignature = new RootSignature;
			m_GlobalRootSignature->InitCustomize(TotalTableNum, RootParameters, nullptr);

			SubObject.Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
			ID3D12RootSignature* RootSign = m_GlobalRootSignature->GetRootSignature().Get();
			SubObject.pDesc = &RootSign;

#ifdef _DEBUG
			SubObjectArray[index] = SubObject;
#else
			SubObjectArray.push_back(SubObject);
#endif
			index++;

			m_DescriptorTableData.clear();
			m_DescriptorTableData.shrink_to_fit();
		}

		// PSO(RT)作成
		{
			D3D12_STATE_OBJECT_DESC* StateObjectDesc = new D3D12_STATE_OBJECT_DESC;
			ZeroMemory(StateObjectDesc, sizeof(D3D12_STATE_OBJECT_DESC));
			StateObjectDesc->Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
			StateObjectDesc->NumSubobjects = index;
			StateObjectDesc->pSubobjects = SubObjectArray.data();

			hr = m_Device->CreateStateObject(StateObjectDesc, IID_PPV_ARGS(&m_PSO));
			if (FAILED(hr))
			{
				throw std::runtime_error("CreatePSO(RT)Failed");
			}

			EntryPoints.clear();
			EntryPoints.shrink_to_fit();
		}
	}

	//シェーダーテーブル作成
	//0-RayGenシェーダー　 1-Missシェーダー 　2-ClosestHitシェーダー
	{
		m_ShaderTable = new ShaderTable;
		m_ShaderTable->Init(1 + TotalMissShader + TotalHitShader);

		m_ShaderTable->Update(m_PSO, RayGenShaderName,
			MissShaderName, TotalMissShader,
			HitGroupName, TotalHitShader);
	}

	//出力用バッファ(テクスチャ)の作成 0
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;	//UAVとして
		desc.Width = SCREEN_WIDTH;
		desc.Height = SCREEN_HEIGHT;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
		auto HeapProp = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		HRESULT hr = m_Device->CreateCommittedResource(&HeapProp, D3D12_HEAP_FLAG_NONE, &desc,
			D3D12_RESOURCE_STATE_COPY_SOURCE, nullptr, IID_PPV_ARGS(&m_OutputBuffer));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateOutputBufferFailed");
		}

		m_OutputBuffer->SetName(L"OutputBuffer");

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAV_Desc = {};
		UAV_Desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
		m_Device->CreateUnorderedAccessView(m_OutputBuffer.Get(), nullptr, &UAV_Desc,
				handle);
		
	}

	//TLAS View 1
	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRV_Desc = {};
		SRV_Desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		SRV_Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRV_Desc.RaytracingAccelerationStructure.Location = m_TLASBuffers->GetResultBuffer()->GetGPUVirtualAddress();
		
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += 1 * m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_Device->CreateShaderResourceView(nullptr, &SRV_Desc, handle);
	}

	//Camera 2
	{
		m_CameraBuffer = new ConstantBuffer;
		XMFLOAT3 CameraPos = XMFLOAT3(0.0f, 8.0f, -25.0f);
		XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		CameraData.CameraRot = XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
		CameraData.pos = CameraPos;
		CameraData.f_near = 0;
		CameraData.f_far = 1000;
		CameraData.aspect = 1;

		m_CameraBuffer->Init(sizeof(CameraRT), &CameraData);

		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += 2 * m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		m_CameraBuffer->CreateConstantBufferView(handle);
	}
}

void RayTracingControl::UnInit()
{
	//m_OutputBuffer->Release();
	m_CameraBuffer->UnInit();
	
	
	
#pragma omp parallel for
	for (int i = 0; i < sizeof(ClosestHitShaderName) / sizeof(*ClosestHitShaderName); i++)
	{
		m_DxilLibrary[i]->UnInit();
	}
	m_RayGenLocal->Release();
	m_GlobalRootSignature->UnInit();
	
	m_PSO->Release();
	m_ShaderTable->UnInit();

	m_TLASBuffers->UnInit();
	//m_DescHeap->Release();
	
}

void RayTracingControl::Update()
{
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
	auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
	Scene* scene = GameManager::GetScene();
	static XMFLOAT3 cameraRot = XMFLOAT3(0.4f, 0.0f, 0.0f);

	float speed = 0.05f;

	if (Input::GetKeyPress('A'))
	{
		XMVECTOR right = XMVector3Normalize(XMVector3Cross(CameraData.CameraRot.r[2], XMVectorSet(0, 1, 0, 0)));
		XMFLOAT3 right3;
		XMStoreFloat3(&right3, right);
		CameraData.pos.x = CameraData.pos.x + right3.x * speed;
		CameraData.pos.y = CameraData.pos.y + right3.y * speed;
		CameraData.pos.z = CameraData.pos.z + right3.z * speed;
	}

	if (Input::GetKeyPress('D'))
	{
		XMVECTOR right = XMVector3Normalize(XMVector3Cross(CameraData.CameraRot.r[2], XMVectorSet(0, 1, 0, 0)));
		XMFLOAT3 right3;
		XMStoreFloat3(&right3, right);
		CameraData.pos.x = CameraData.pos.x - right3.x * speed;
		CameraData.pos.y = CameraData.pos.y - right3.y * speed;
		CameraData.pos.z = CameraData.pos.z - right3.z * speed;
	}

	XMVECTOR posVec = XMLoadFloat3(&CameraData.pos);

	if (Input::GetKeyPress('W'))
	{
		posVec += CameraData.CameraRot.r[2] * speed;
	}

	if (Input::GetKeyPress('S'))
	{
		posVec -= CameraData.CameraRot.r[2] * speed;
	}

	XMFLOAT3 newPos;
	XMStoreFloat3(&newPos, posVec);
	CameraData.pos = newPos;

	if (Input::GetKeyPress('Q') || Input::GetKeyPress(VK_LEFT))
	{
		cameraRot.y -= speed / 2;
	}

	if (Input::GetKeyPress('E') || Input::GetKeyPress(VK_RIGHT))
	{
		cameraRot.y += speed / 2;
	}

	if (Input::GetKeyPress(VK_UP))
	{
		cameraRot.x -= speed;
	}

	if (Input::GetKeyPress(VK_DOWN))
	{
		cameraRot.x += speed;
	}

	// カメラの向きを更新する
	CameraData.CameraRot = XMMatrixRotationRollPitchYaw(cameraRot.x, cameraRot.y, cameraRot.z);

	m_CameraBuffer->Update(CameraData);

	//Update TLAS
	{
		//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
		m_CommandAllocator->Reset();
		//コマンドリストをリセットする
		m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

		Scene* scene = GameManager::GetScene();
		int TotalObjectNum = 0;
#pragma omp parallel for
		for (auto object : scene->GetRTObjectList())
		{
			TotalObjectNum += object->GetInitObjectNum();
		}
		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
		inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
		inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
		inputs.NumDescs = TotalObjectNum;
		inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
		m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);

		//UAVバリア
		{
			D3D12_RESOURCE_BARRIER UavBarrier = {};
			UavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			UavBarrier.UAV.pResource = m_TLASBuffers->GetResultBuffer().Get();
			m_CommandList->ResourceBarrier(1, &UavBarrier);
		}
		

		D3D12_RAYTRACING_INSTANCE_DESC* InstanceDesc;
		m_TLASBuffers->GetInstanceDescBuffer()->Map(0, nullptr, (void**)&InstanceDesc);

#pragma omp parallel for
		for (int i = 0; i < TotalObjectNum; i++)
		{
			static int id = 0;

			InstanceDesc[id] = scene->GetInstanceContainer()[id];

			id++;

			if (id >= TotalObjectNum)
				id = 0;
		}

		m_TLASBuffers->GetInstanceDescBuffer()->Unmap(0, nullptr);

		D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC AS_Desc = {};
		AS_Desc.Inputs = inputs;
		AS_Desc.Inputs.InstanceDescs = m_TLASBuffers->GetInstanceDescBuffer()->GetGPUVirtualAddress();
		AS_Desc.DestAccelerationStructureData = m_TLASBuffers->GetResultBuffer()->GetGPUVirtualAddress();
		AS_Desc.ScratchAccelerationStructureData = m_TLASBuffers->GetScratchBuffer()->GetGPUVirtualAddress();

		AS_Desc.Inputs.Flags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
		AS_Desc.SourceAccelerationStructureData = m_TLASBuffers->GetResultBuffer()->GetGPUVirtualAddress();

		m_CommandList->BuildRaytracingAccelerationStructure(&AS_Desc, 0, nullptr);

		//UAVバリア
		{
			D3D12_RESOURCE_BARRIER UavBarrier = {};
			UavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
			UavBarrier.UAV.pResource = m_TLASBuffers->GetResultBuffer().Get();
			m_CommandList->ResourceBarrier(1, &UavBarrier);
		}

		//scene->GetInstanceContainer().clear();

		m_CommandList->Close();
		ComPtr<ID3D12CommandList> ppCommandLists[] = { m_CommandList.Get() };
		m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists->GetAddressOf());

		GetDX12Renderer->WaitGpu();

		m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);
		m_CommandList->Close();
	}
}

void RayTracingControl::Begin()
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_Device = GetDX12Renderer->GetDevice();

	ID3D12DescriptorHeap* ppHeaps[] = { m_DescHeap.Get() };
	m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	if (TotalTableNum >= 1)
	{
		m_CommandList->SetComputeRootSignature(m_GlobalRootSignature->GetRootSignature().Get());

		UINT Stride = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#pragma omp parallel for
		for (int i = 0; i < TotalTableNum; i++)
		{
			CD3DX12_GPU_DESCRIPTOR_HANDLE srvHandle(m_DescHeap->GetGPUDescriptorHandleForHeapStart(), TableStart[i], Stride);
			m_CommandList->SetComputeRootDescriptorTable(i, srvHandle);
		}
	}
}

void RayTracingControl::End()
{
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_Device = GetDX12Renderer->GetDevice();
	auto m_RTarget = GetDX12Renderer->GetRTarget(GetDX12Renderer->GetBackBufferIndex());
	Scene* scene = GameManager::GetScene();
	const UINT TotalHitShader = sizeof(ClosestHitShaderName) / sizeof(*ClosestHitShaderName);
	const UINT TotalMissShader = sizeof(MissShaderName) / sizeof(*MissShaderName);

	//出力バッファのバリア
	GetDX12Renderer->CreateOutputBarrier(m_OutputBuffer);

	//レイトレースに必要な情報を記述していく
	D3D12_DISPATCH_RAYS_DESC RaytraceDesc = {};
	RaytraceDesc.Width = SCREEN_WIDTH;
	RaytraceDesc.Height = SCREEN_HEIGHT;
	RaytraceDesc.Depth = 1;

	int ShaderTableEntrySize = m_ShaderTable->GetEntrySize();

	// RayGenのシェーダーテーブル内の位置
	RaytraceDesc.RayGenerationShaderRecord.StartAddress = m_ShaderTable->GetGPUAddress();
	RaytraceDesc.RayGenerationShaderRecord.SizeInBytes = ShaderTableEntrySize;

	// Missのシェーダーテーブル内の位置
	int MissOffset = 1 * ShaderTableEntrySize;
	D3D12_GPU_VIRTUAL_ADDRESS MissAddress = m_ShaderTable->GetGPUAddress() + MissOffset;
	RaytraceDesc.MissShaderTable.StartAddress = MissAddress;
	RaytraceDesc.MissShaderTable.StrideInBytes = ShaderTableEntrySize;
	RaytraceDesc.MissShaderTable.SizeInBytes = ShaderTableEntrySize * TotalMissShader;

	// ClosestHitのシェーダーテーブル内の位置
	int HitOffset = (1 + TotalMissShader) * ShaderTableEntrySize;
	D3D12_GPU_VIRTUAL_ADDRESS HitAddress = m_ShaderTable->GetGPUAddress() + HitOffset;
	RaytraceDesc.HitGroupTable.StartAddress = HitAddress;
	RaytraceDesc.HitGroupTable.StrideInBytes = ShaderTableEntrySize;
	RaytraceDesc.HitGroupTable.SizeInBytes = ShaderTableEntrySize * TotalHitShader;

	scene->GetInstanceContainer().clear();

	//ディスパッチ（レイトレース実行）
	m_CommandList->SetPipelineState1(m_PSO.Get());
	m_CommandList->DispatchRays(&RaytraceDesc);

	////Outputバッファをコピー
	GetDX12Renderer->CopyResource(m_OutputBuffer);

	////バリアのトランジションを更新する
	GetDX12Renderer->ChangeTransition(m_RTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);
}