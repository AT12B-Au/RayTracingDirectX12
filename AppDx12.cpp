#include "main.h"
#include "AppDx12.h"
#include "RendererManager.h"
#include "RootSignature.h"
#include "ImGui12.h"
#include <dxgi1_6.h>

//Direct3D12初期化
HRESULT AppDx12::Init()
{
	HRESULT hr;

	//デバッグレイヤー Dx12での開発においてこれは必需品
	UINT DxgiFactoryFlags = 0;
#if defined(_DEBUG)	
	
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController12))))
	{
		DebugController12->EnableDebugLayer();
		DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	
#if 0 // GBV を有効化する場合.
		ComPtr<ID3D12Debug3> debug3;
		debug.As(&debug3);
		if (debug3)
		{
			debug3->SetEnableGPUBasedValidation(true);
		}
#endif
	}
#endif
	//ファクトリー作成
	ComPtr<IDXGIFactory4> factory;
	{
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateFactoryFailed");
		}
	}	

	
	
	ComPtr<IDXGIFactory6> factory6;
	hr = factory.As(&factory6);
	if (FAILED(hr))
	{
		throw std::runtime_error("DXGI 1.6 not supported");
	}
	
	// ハードウェアアダプタの検索
	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIAdapter1> UseAdapter;

	for (UINT adapterID = 0; DXGI_ERROR_NOT_FOUND != factory6->EnumAdapterByGpuPreference(adapterID, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)); ++adapterID)
	{
		DXGI_ADAPTER_DESC1 desc;
		hr = adapter->GetDesc1(&desc);
		if (FAILED(hr))
		{
			throw std::runtime_error("CannotGetAdapterDesc");
		}

		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			break;
		}
	}

	if (!adapter)
	{
		throw std::runtime_error("UnavailableAdapter");
	}

	UseAdapter = adapter.Detach();

	ComPtr<ID3D12Device> testDevice;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

	hr = D3D12CreateDevice(UseAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&testDevice));
	if (FAILED(hr))
	{
		throw std::runtime_error("TestDeviceFailed");
	}
	hr = testDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &featureSupportData, sizeof(featureSupportData));
	if (FAILED(hr))
	{
		throw std::runtime_error("DeviceFeatureNotSupport");
	}

	if (featureSupportData.RaytracingTier == D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
	{
		throw std::runtime_error("GraphicCardNotSuppotRayTracing");
	}

	hr = D3D12CreateDevice(UseAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateDeviceFailed");
	}

	//bool GraphicDebugging = false;
	//bool CheckGPU = false;
	//ComPtr<ID3D12Device8> CheckDevice;
	//#pragma omp parallel for
	//for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	//{
	//	DXGI_ADAPTER_DESC1 desc;
	//	adapter->GetDesc1(&desc);

	//	if (wcsstr(desc.Description, L"Capture Adapter") != nullptr) {}
	//	else if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
	//		continue;

	//	// D3D12は使用可能か
	//	hr = D3D12CreateDevice(
	//		adapter.Get(),
	//		D3D_FEATURE_LEVEL_12_1,
	//		IID_PPV_ARGS(&CheckDevice));
	//	if (SUCCEEDED(hr))
	//	{
	//		D3D12_FEATURE_DATA_D3D12_OPTIONS5 features5;
	//		hr = CheckDevice->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &features5, sizeof(D3D12_FEATURE_DATA_D3D12_OPTIONS5));
	//		if (features5.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED)
	//		{
	//				adapter.As(&UseAdapter); // 使用するアダプター
	//				CheckGPU = true;
	//				break;	
	//		}
	//	}
	//	if (FAILED(hr))
	//	{
	//		throw std::runtime_error("CreateAdapterFailed");
	//	}
	//}
	//if (!CheckGPU)
	//{
	//	MessageBoxA(0, "グラフィックカードがRayTracingに対応しません", "起動エラー", MB_ICONERROR);
	//	exit(0);
	//}
	//	

	////DX12デバイス作成
	//hr = D3D12CreateDevice(UseAdapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_Device));
	//if (FAILED(hr))
	//{
	//	throw std::runtime_error("CreateDeviceFailed");
	//}

	m_RtvStride = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_DsvStride = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//m_cbvSrvStride = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	//コマンドキュー作成
	{
		D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
		QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		hr = m_Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&m_CommandQueue));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateCommandQueueFailed");
		}
	}
	//コマンドリスト作成
	{
		hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateCommandAllocatorFailed");
		}
		hr = m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), NULL, IID_PPV_ARGS(&m_CommandList));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateCommandListFailed");
		}

		m_CommandList->Close();
	}
	//スワップチェーン作成
	{
		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
		SwapChainDesc.BufferCount = FRAME_COUNT;
		SwapChainDesc.Width = SCREEN_WIDTH;
		SwapChainDesc.Height = SCREEN_HEIGHT;
		SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		SwapChainDesc.SampleDesc.Count = 1;
		SwapChainDesc.SampleDesc.Quality = 0;
		SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		
		ComPtr<IDXGISwapChain1> SwapChain;
		hr = factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), WindowMain::m_HWnd, &SwapChainDesc, NULL, NULL, &SwapChain);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateSwapChainFailed");
		}

		SwapChain.As(&m_SwapChain);
	}
	//レンダーターゲット作成
	{
		//レンダーターゲットビューのヒープ作成
		D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDesc = {};
		RtvHeapDesc.NumDescriptors = FRAME_COUNT;
		RtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		RtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		hr = m_Device->CreateDescriptorHeap(&RtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateRtvHeapFailed");
		}

		//レンダーターゲットビュー作成
		CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());

		#pragma omp parallel for
		for (UINT n = 0; n < FRAME_COUNT; n++)
		{
			m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_RenderTargets[n]));
			m_Device->CreateRenderTargetView(m_RenderTargets[n].Get(), NULL, RtvHandle);
			RtvHandle.Offset(1, m_RtvStride);
			//SetName
			m_RenderTargets[n]->SetName(L"RenderTargets");
		}
	}
	//深度バッファ
	{
		//深度バッファ自体の作成
		D3D12_HEAP_PROPERTIES DsHeapProp;
		ZeroMemory(&DsHeapProp, sizeof(&DsHeapProp));
		DsHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		DsHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		DsHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		DsHeapProp.CreationNodeMask = NULL;
		DsHeapProp.VisibleNodeMask = NULL;

		D3D12_RESOURCE_DESC DsResDesc;
		ZeroMemory(&DsResDesc, sizeof(D3D12_RESOURCE_DESC));
		DsResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		//DsResDesc.Alignment = 0;
		DsResDesc.Width = SCREEN_WIDTH;
		DsResDesc.Height = SCREEN_HEIGHT;
		DsResDesc.DepthOrArraySize = 1;
		DsResDesc.MipLevels = 1;
		DsResDesc.Format = DXGI_FORMAT_D32_FLOAT;
		DsResDesc.SampleDesc.Count = 1;
		DsResDesc.SampleDesc.Quality = 0;
		DsResDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		DsResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue;
		ZeroMemory(&clearValue, sizeof(D3D12_CLEAR_VALUE));
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_Device->CreateCommittedResource(
			&DsHeapProp,
			D3D12_HEAP_FLAG_NONE,
			&DsResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(&m_DepthBuffer)
		);

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateDepthBufferFailed");
		}

		//SetName
		m_DepthBuffer->SetName(L"DepthBuffer");

		//深度バッファのビュー用のヒープ
		D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc;
		ZeroMemory(&DsvHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
		DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DsvHeapDesc.NumDescriptors = 1;
		DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		DsvHeapDesc.NodeMask = NULL;
		hr = m_Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(&m_DspHeap));

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateDspHeapFailed");
		}

		//深度バッファのビュー
		D3D12_DEPTH_STENCIL_VIEW_DESC DsViewDesk;
		ZeroMemory(&DsViewDesk, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
		DsViewDesk.Format = DXGI_FORMAT_D32_FLOAT;
		DsViewDesk.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		DsViewDesk.Flags = D3D12_DSV_FLAG_NONE;
		DsViewDesk.Texture2D.MipSlice = 0;
		D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = m_DspHeap->GetCPUDescriptorHandleForHeapStart();

		m_Device->CreateDepthStencilView(m_DepthBuffer.Get(), &DsViewDesk, DsvHandle);
	}

	//フェンス作成
	{
		hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateFenceFailed");
		}
		m_FenceValue = 1;
	}

	//ルートシグネチャ
	{
		m_RootSignature = new RootSignature;
		m_RootSignature->InitNormal();
	}

	//ImGui
	{
		m_ImGuiHeap = CreateImGuiHeap();
		if (m_ImGuiHeap == nullptr)
			return E_FAIL;
	}

	return S_OK;
}

void AppDx12::Update()
{

}

//
//シーンを画面にレンダリング
void AppDx12::Begin()
{
	//バックバッファが現在何枚目かを取得
	UINT BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
	m_CommandAllocator->Reset();
	//コマンドリストをリセットする
	m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

	//ビューポートをセット
	CD3DX12_VIEWPORT  viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)SCREEN_WIDTH, (float)SCREEN_HEIGHT);
	CD3DX12_RECT  ScissorRect = CD3DX12_RECT(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	m_CommandList->RSSetViewports(1, &viewport);
	m_CommandList->RSSetScissorRects(1, &ScissorRect);

	//バックバッファのトランジションをレンダーターゲットモードにする
	auto tran = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[BackBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_CommandList->ResourceBarrier(1, &tran);

	//バックバッファをレンダーターゲットにセット
	CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart(), BackBufferIndex, m_RtvStride);
	D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = m_DspHeap->GetCPUDescriptorHandleForHeapStart();
	m_CommandList->OMSetRenderTargets(1, &RtvHandle, false, &DsvHandle);
	
	//画面クリア
	const float ClearColorLocal[] = { ImGui12::ClearColor.x * ImGui12::ClearColor.w , ImGui12::ClearColor.y * ImGui12::ClearColor.w, ImGui12::ClearColor.z * ImGui12::ClearColor.w, ImGui12::ClearColor.y * ImGui12::ClearColor.w };
	m_CommandList->ClearRenderTargetView(RtvHandle, ClearColorLocal, 0, NULL);
	m_CommandList->ClearDepthStencilView(DsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, NULL);

	//ルートシグネチャをセット
	m_CommandList->SetGraphicsRootSignature(m_RootSignature->GetRootSignature().Get());
}

void AppDx12::End()
{
	//バックバッファが現在何枚目かを取得
	UINT BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	auto tran = CD3DX12_RESOURCE_BARRIER::Transition(m_RenderTargets[BackBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

	m_CommandList->ResourceBarrier(1, &tran);
	//コマンドの書き込みはここで終わり、Closeする
	m_CommandList->Close();

	//コマンドリストの実行
	ComPtr<ID3D12CommandList> ppCommandLists[] = { m_CommandList.Get() };
	m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists->GetAddressOf());

	//バックバッファをフロントバッファに切り替えてシーンをモニターに表示
	m_SwapChain->Present(1, 0);

	//GPUの処理が完了するまで待つ
	WaitGpu();
}

//
//同期処理　Gpuの処理が完了するまで待つ
void AppDx12::WaitGpu()
{
	//GPUサイドが全て完了したときにGPUサイドから返ってくる値（フェンス値）をセット
	m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue);

	//上でセットしたシグナルがGPUから帰ってくるまでストール（この行で待機）
	do
	{
		//GPUの完了を待つ間、ここで何か有意義な事（CPU作業）をやるほど効率が上がる

	} while (m_Fence->GetCompletedValue() < m_FenceValue);

	//ここでフェンス値を更新する 前回より大きな値であればどんな値でもいいわけだが、1足すのが簡単なので1を足す
	m_FenceValue++;
}

void AppDx12::UnInit()
{
	m_RootSignature->UnInit();
	m_ImGuiHeap->Release();
	m_Fence->Release();
	m_DspHeap->Release();
	m_RtvHeap->Release();

	#pragma omp parallel for
	for (int i = 0; i < FRAME_COUNT; i++)
	{
		m_RenderTargets[i]->Release();
	}

	m_DepthBuffer->Release();
	m_SwapChain->Release();
	m_CommandAllocator->Release();
	m_CommandList->Release();
	m_CommandQueue->Release();
	m_Device->Release();
}

ComPtr<ID3D12Resource> AppDx12::CreateVertexBuffer(UINT m_VertexBufferSize)
{
	HRESULT hr;
	ComPtr<ID3D12Resource> m_vertexBuffer;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	auto re_desc = CD3DX12_RESOURCE_DESC::Buffer(m_VertexBufferSize);

	//バーテックスバッファ作成
	hr = m_Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&re_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_vertexBuffer));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateVertexBufferFailed");
	}

	return m_vertexBuffer;
}

ComPtr<ID3D12Resource> AppDx12::CreateVertexBufferUpLoad(UINT m_VertexBufferSize)
{
	HRESULT hr;
	ComPtr<ID3D12Resource> m_vertexBufferUpLoad;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto re_desc = CD3DX12_RESOURCE_DESC::Buffer(m_VertexBufferSize);

	//バーテックスバッファ作成
	hr = m_Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&re_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_vertexBufferUpLoad));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateVertexBufferFailed");
	}

	return m_vertexBufferUpLoad;
}

D3D12_VERTEX_BUFFER_VIEW AppDx12::CreateVertexBufferView(ComPtr<ID3D12Resource> m_VertexBuffer, UINT m_VertexBufferSize)
{
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	//バーテックスバッファビュー作成
	m_vertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
	m_vertexBufferView.StrideInBytes = sizeof(VERTEX_3D12);
	m_vertexBufferView.SizeInBytes = m_VertexBufferSize;

	return m_vertexBufferView;
}

ComPtr<ID3D12Resource> AppDx12::CreateConstantBuffer(UINT m_CBSize)
{
	HRESULT hr;
	ComPtr<ID3D12Resource> m_constantBuffer;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto re_desc = CD3DX12_RESOURCE_DESC::Buffer(m_CBSize);
	hr = m_Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&re_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_constantBuffer));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateConstantBufferFailed");
	}

	return m_constantBuffer;
}

ComPtr<ID3D12DescriptorHeap> AppDx12::CreateConstantBufferViewHeap(UINT num)
{
	HRESULT hr;
	ComPtr<ID3D12DescriptorHeap> m_DescHeap;

	//コンスタントバッファビューのヒープ作成
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
	cbvHeapDesc.NumDescriptors = num;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = m_Device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_DescHeap));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateConstantViewHeapFailed");
	}

	return m_DescHeap;
}

D3D12_CONSTANT_BUFFER_VIEW_DESC AppDx12::CreateConstantBufferView(ComPtr<ID3D12Resource> m_ConstantBuffer,ComPtr<ID3D12DescriptorHeap> m_DescHeap,UINT num,UINT CBSize)
{
	////コンスタントバッファビュー作成
	//UINT CBSize;
	//CBSize = (sizeof(GlobalBuffer12) + 255) & ~255;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = m_ConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = CBSize;
	D3D12_CPU_DESCRIPTOR_HANDLE cHandle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
	//UINT stride = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	cHandle.ptr += m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * num;
	m_Device->CreateConstantBufferView(&cbvDesc, cHandle);

	return cbvDesc;
}

ComPtr<ID3D12Resource> AppDx12::CreateTexture(DXGI_FORMAT format,UINT width,UINT height)
{
	ComPtr<ID3D12Resource> m_texture;

	//まず空白のテクスチャーを作成する
	D3D12_RESOURCE_DESC tdesc;
	ZeroMemory(&tdesc, sizeof(tdesc));
	tdesc.MipLevels = 1;
	if (format)
		tdesc.Format = format;
	else
	{
		throw std::runtime_error("NoTextureFormat");
	}
	
	tdesc.Width = width;
	tdesc.Height = height;
	tdesc.Flags = D3D12_RESOURCE_FLAG_NONE;
	tdesc.DepthOrArraySize = 1;
	tdesc.SampleDesc.Count = 1;
	tdesc.SampleDesc.Quality = 0;
	tdesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

	m_Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&tdesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_texture));

	return m_texture;
}

ComPtr<ID3D12Resource> AppDx12::CreateTextureUpLoad(ComPtr<ID3D12Resource> m_Texture)
{
	ComPtr<ID3D12Resource> m_textureUpLoad;

	//CPUからGPUへテクスチャーデータを渡すときの中継としてのテクスチャーを作る
	UINT64 BufferSize = GetRequiredIntermediateSize(m_Texture.Get(), 0, 1);

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto re_desc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);

	m_Device->CreateCommittedResource(
		&prop,//ヒープタイプはCPUから書き込み可能なUPLOADHEAP
		D3D12_HEAP_FLAG_NONE,
		&re_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_textureUpLoad));

	return m_textureUpLoad;
}

ComPtr<ID3D12Resource> AppDx12::CreateIndexBuffer(UINT m_IndexBufferSize)
{
	HRESULT hr;
	ComPtr<ID3D12Resource> m_indexBuffer;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto re_desc = CD3DX12_RESOURCE_DESC::Buffer(m_IndexBufferSize);

	hr = m_Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&re_desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&m_indexBuffer));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIndexBufferFailed");
	}

	return m_indexBuffer;
}

void AppDx12::CreateTextureView(ComPtr<ID3D12Resource> m_Texture, ComPtr<ID3D12DescriptorHeap> m_DescHeap,DXGI_FORMAT format,UINT NumDescriptors)
{
	if (format == NULL)
	{
		throw std::runtime_error("NoTextureFormat");
	}

	//このテクスチャーのビュー（SRV)を作る
	D3D12_SHADER_RESOURCE_VIEW_DESC sdesc = {};
	sdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	sdesc.Format = format;
	sdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	sdesc.Texture2D.MipLevels = 1;
	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * NumDescriptors;
	m_Device->CreateShaderResourceView(m_Texture.Get(), &sdesc, handle);
}

void AppDx12::CreateShadowView(ComPtr<ID3D12DescriptorHeap> m_DescHeap, UINT NumDescriptors)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC sdesc{};
	sdesc.Format = DXGI_FORMAT_R32_FLOAT;
	sdesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	sdesc.Texture2D.MipLevels = 1;
	sdesc.Texture2D.MostDetailedMip = 0;
	sdesc.Texture2D.PlaneSlice = 0;
	sdesc.Texture2D.ResourceMinLODClamp = 0.0F;
	sdesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	D3D12_CPU_DESCRIPTOR_HANDLE handle = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * NumDescriptors;
	m_Device->CreateShaderResourceView(m_ShadowBuffer.Get(), &sdesc, handle);
}

ComPtr<ID3D12Resource> AppDx12::CreateIndexBufferUpLoad(UINT m_IndexBufferSize)
{
	HRESULT hr;

	ComPtr<ID3D12Resource> m_indexBufferUpLoad;

	auto prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto re_desc = CD3DX12_RESOURCE_DESC::Buffer(m_IndexBufferSize);

	hr = m_Device->CreateCommittedResource(
		&prop,
		D3D12_HEAP_FLAG_NONE,
		&re_desc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&m_indexBufferUpLoad));

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateIndexBufferUpLoadFailed");
	}

	return m_indexBufferUpLoad;
}

D3D12_INDEX_BUFFER_VIEW AppDx12::CreateIndexBufferView(ComPtr<ID3D12Resource> m_IndexBuffer, UINT m_IndexBufferSize)
{
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	//インデックスバッファビュー作成
	m_indexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
	m_indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_indexBufferView.SizeInBytes = m_IndexBufferSize;

	return m_indexBufferView;
}

ComPtr<ID3D12PipelineState> AppDx12::CreatePipelineState(ComPtr<ID3DBlob> m_VertexShader, ComPtr<ID3DBlob> m_PixelShader,ComPtr<ID3D12RootSignature> m_RootSignature)
{
	HRESULT hr;

	//頂点レイアウト作成
	D3D12_INPUT_ELEMENT_DESC m_vertexLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 6, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 10, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDesc = {};
	PsoDesc.InputLayout = { m_vertexLayout, _countof(m_vertexLayout) };
	PsoDesc.pRootSignature = m_RootSignature.Get();
	PsoDesc.VS = CD3DX12_SHADER_BYTECODE(m_VertexShader.Get());
	PsoDesc.PS = CD3DX12_SHADER_BYTECODE(m_PixelShader.Get());
	PsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	PsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	PsoDesc.RasterizerState.DepthClipEnable = true;
	PsoDesc.RasterizerState.MultisampleEnable = true;
	PsoDesc.RasterizerState.FrontCounterClockwise = false;
	PsoDesc.RasterizerState.AntialiasedLineEnable = false;
	PsoDesc.RasterizerState.ForcedSampleCount = 0;
	PsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	PsoDesc.BlendState.AlphaToCoverageEnable = false;	//半透明
	PsoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	PsoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	PsoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	PsoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	PsoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	PsoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	PsoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	PsoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	PsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);

	const D3D12_DEPTH_STENCILOP_DESC DefaultStencilOp =
	{ D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	PsoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	PsoDesc.DepthStencilState.DepthEnable = true;
	PsoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	PsoDesc.DepthStencilState.FrontFace = DefaultStencilOp;
	PsoDesc.DepthStencilState.BackFace = DefaultStencilOp;
	PsoDesc.DepthStencilState.StencilEnable = false;
	PsoDesc.SampleMask = UINT_MAX;
	PsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	PsoDesc.NumRenderTargets = 1;
	PsoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	PsoDesc.SampleDesc.Count = 1;
	PsoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	ComPtr<ID3D12PipelineState> m_PipelineState;

	hr = m_Device->CreateGraphicsPipelineState(&PsoDesc, IID_PPV_ARGS(&m_PipelineState));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreatePipelineStateFailed");
	}

	return m_PipelineState;
}

void AppDx12::CreateOutputBarrier(ComPtr<ID3D12Resource> OutputBuffer)
{
	D3D12_RESOURCE_BARRIER OutputBarrier = {};
	OutputBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	OutputBarrier.Transition.pResource = OutputBuffer.Get();
	OutputBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	OutputBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	OutputBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	m_CommandList->ResourceBarrier(1, &OutputBarrier);
}

void AppDx12::CopyResource(ComPtr<ID3D12Resource> resource)
{
	D3D12_RESOURCE_BARRIER barrier = {};
	//バックバッファが現在何枚目かを取得
	UINT BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	//それをスワップチェーンバッファにコピーして画面に表示する
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	m_CommandList->ResourceBarrier(1, &barrier);

	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = m_RenderTargets[BackBufferIndex].Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	m_CommandList->ResourceBarrier(1, &barrier);

	m_CommandList->CopyResource(m_RenderTargets[BackBufferIndex].Get(), resource.Get());
}

void AppDx12::ChangeTransition(ComPtr<ID3D12Resource> resource,D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
{
	//バックバッファが現在何枚目かを取得
	UINT BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = resource.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = before;
	barrier.Transition.StateAfter = after;
	m_CommandList->ResourceBarrier(1, &barrier);
}

UINT AppDx12::GetBackBufferIndex()
{
	return m_SwapChain->GetCurrentBackBufferIndex();
}

XMMATRIX AppDx12::SetWorldViewProjection2D(UINT num)
{
	if (num == 0)
	{
		XMMATRIX world;
		world = XMMatrixIdentity();
	
		return world;
	}
	else if (num == 1)
	{
		XMMATRIX view;
		view = XMMatrixIdentity();
	
		return view;
	}
	else if (num == 2)
	{
		XMMATRIX projection;
		projection = XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
		return projection;
	}
	else
	{
		throw std::runtime_error("WrongNum");
	}
}

void AppDx12::SetGlobalBuffer(GlobalBuffer12* gb, ComPtr<ID3D12Resource> GlobalBuffer)
{
	CD3DX12_RANGE readRange(0, 0);
	UINT8* pCbvDataBegin;

	HRESULT hr;

	//ワールドマトリクス更新
	hr = GlobalBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin));
	if (FAILED(hr))
	{
		throw std::runtime_error("SetGlobalBufferFailed");
	}

	memcpy(pCbvDataBegin, gb, sizeof(GlobalBuffer12));
	GlobalBuffer->Unmap(0, nullptr);
}

void AppDx12::SetWorldMatrix(XMMATRIX WorldMatrix, ComPtr<ID3D12Resource> WorldBuffer)
{
	CD3DX12_RANGE readRange(0, 0);
	UINT8* pCbvDataBegin;

	HRESULT hr;
	XMMATRIX world;

	//ワールドマトリクス更新
	hr = WorldBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin));
	if (FAILED(hr))
	{
		throw std::runtime_error("SetWorldMatrixFailed");
	}
	world = XMMatrixTranspose(WorldMatrix);
	memcpy(pCbvDataBegin, &world, sizeof(XMMATRIX));
	WorldBuffer->Unmap(0, nullptr);
}

void AppDx12::SetViewMatrix(XMMATRIX ViewMatrix, ComPtr<ID3D12Resource> ViewBuffer)
{
	CD3DX12_RANGE readRange(0, 0);
	UINT8* pCbvDataBegin;

	HRESULT hr;
	XMMATRIX view;

	//ワールドマトリクス更新
	hr = ViewBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin));
	if (FAILED(hr))
	{
		throw std::runtime_error("SetViewMatrixFailed");
	}
	view = XMMatrixTranspose(ViewMatrix);
	memcpy(pCbvDataBegin, &view, sizeof(XMMATRIX));
	ViewBuffer->Unmap(0, nullptr);
}

void AppDx12::SetProjectMatrix(XMMATRIX ProjectionMatrix, ComPtr<ID3D12Resource> ProjectionBuffer)
{
	CD3DX12_RANGE readRange(0, 0);
	UINT8* pCbvDataBegin;

	HRESULT hr;
	XMMATRIX projection;

	//ワールドマトリクス更新
	hr = ProjectionBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin));
	if (FAILED(hr))
	{
		throw std::runtime_error("SetProjectionMatrixFailed");
	}
	projection = XMMatrixTranspose(ProjectionMatrix);
	memcpy(pCbvDataBegin, &projection, sizeof(XMMATRIX));
	ProjectionBuffer->Unmap(0, nullptr);
}

void AppDx12::SetLight(LIGHT12* light, ComPtr<ID3D12Resource> LightBuffer)
{
	CD3DX12_RANGE readRange(0, 0);
	UINT8* pCbvDataBegin;

	HRESULT hr;

	//ワールドマトリクス更新
	hr = LightBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pCbvDataBegin));
	if (FAILED(hr))
	{
		throw std::runtime_error("SetLightMatrixFailed");
	}

	memcpy(pCbvDataBegin, light, sizeof(LIGHT12));
	LightBuffer->Unmap(0, nullptr);
}

ComPtr<ID3DBlob> AppDx12::CreateVertexShader(const wchar_t* ShaderName)
{
	HRESULT hr;

	ComPtr<ID3DBlob> m_VertexShader;

	hr = D3DCompileFromFile(ShaderName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_VertexShader, nullptr);
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateVertexShaderFailed");
	}

	return m_VertexShader;
}

ComPtr<ID3DBlob> AppDx12::CreatePixelShader(const wchar_t* ShaderName)
{
	HRESULT hr;

	ComPtr<ID3DBlob> m_PixelShader;

	hr = D3DCompileFromFile(ShaderName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &m_PixelShader, nullptr);
	if (FAILED(hr))
	{
		throw std::runtime_error("CreatePixelShaderFailed");
	}

	return m_PixelShader;
}

ComPtr<ID3D12DescriptorHeap> AppDx12::CreateImGuiHeap()
{
	HRESULT hr;
	ComPtr<ID3D12DescriptorHeap> ret;
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	hr = m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(ret.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
	{
		throw std::runtime_error("CreateImGuiHeapFailed");
	}
	return ret;
}
