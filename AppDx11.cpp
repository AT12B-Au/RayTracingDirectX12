#include "main.h"
#include "AppDx11.h"
#include <io.h>
#include "ImGui11.h"
#include <dxgi1_4.h>

ComPtr<ID3D11DepthStencilView> AppDx11::m_ShadowDepthStencilView = NULL;
ComPtr<ID3D11ShaderResourceView> AppDx11::m_ShadowDepthShaderResourceView = NULL;

HRESULT AppDx11::Init()
{
	HRESULT hr = S_OK;

	//�f�o�C�X��
	ComPtr<IDXGIFactory4> factory;
	CreateDXGIFactory(__uuidof(IDXGIFactory4), (void**)(&factory));

	// �f�o�C�X�A�X���b�v�`�F�[���쐬
	DXGI_SWAP_CHAIN_DESC SwapChainDesc{};
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Width = SCREEN_WIDTH;
	SwapChainDesc.BufferDesc.Height = SCREEN_HEIGHT;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = WindowMain::m_HWnd;
	SwapChainDesc.SampleDesc.Count = 1;			//MSAA�ݒ�
	SwapChainDesc.SampleDesc.Quality = 0;		//MSAA�ݒ�
	SwapChainDesc.Windowed = TRUE;

	ComPtr<IDXGIAdapter1> adapter;
	ComPtr<IDXGIAdapter1> UseAdapter;

	#pragma omp parallel for
	for (UINT i = 0; factory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		if (wcsstr(desc.Description, L"Capture Adapter") != nullptr) {}
		else if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			continue;

		// D3D12�͎g�p�\��
		hr = D3D11CreateDevice(
			adapter.Get(),
			D3D_DRIVER_TYPE_UNKNOWN,
			NULL, D3D11_CREATE_DEVICE_DEBUG, NULL,0,D3D11_SDK_VERSION,NULL,NULL,NULL);

		adapter.As(&UseAdapter);

		if (FAILED(hr))
			return E_FAIL;
	}

	hr = D3D11CreateDeviceAndSwapChain(UseAdapter.Get(),
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL,
		D3D11_CREATE_DEVICE_DEBUG,
		NULL,
		0,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&m_SwapChain,
		&m_Device,
		&m_FeatureLevel,
		&m_DeviceContext);

	if (adapter != nullptr)
		adapter->Release();

	factory->Release();

	// �����_�[�^�[�Q�b�g�r���[�쐬
	ComPtr<ID3D11Texture2D> renderTarget = NULL;
	m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget);
	m_Device->CreateRenderTargetView(renderTarget.Get(), NULL, &m_RenderTargetView);

	//�X�e���V���p�e�N�X�`���[�쐬
	ComPtr<ID3D11Texture2D> depthStencile = NULL;
	D3D11_TEXTURE2D_DESC textureDesc{};
	textureDesc.Width = SwapChainDesc.BufferDesc.Width;
	textureDesc.Height = SwapChainDesc.BufferDesc.Height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.SampleDesc = SwapChainDesc.SampleDesc;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_Device->CreateTexture2D(&textureDesc, NULL, &depthStencile);

	// �f�v�X�X�e���V���r���[�쐬
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = textureDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;	//MSAA�ݒ�
	depthStencilViewDesc.Flags = 0;
	m_Device->CreateDepthStencilView(depthStencile.Get(), &depthStencilViewDesc, &m_DepthStencilView);


	m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

	// �r���[�|�[�g�ݒ�
	D3D11_VIEWPORT viewport;
	viewport.Width = (FLOAT)SCREEN_WIDTH;
	viewport.Height = (FLOAT)SCREEN_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	m_DeviceContext->RSSetViewports(1, &viewport);



	// ���X�^���C�U�X�e�[�g�ݒ�
	D3D11_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;

	ID3D11RasterizerState* rs;
	m_Device->CreateRasterizerState(&rasterizerDesc, &rs);

	m_DeviceContext->RSSetState(rs);

	rs->Release();


	// �u�����h�X�e�[�g�ݒ�
	D3D11_BLEND_DESC blendDesc = CD3D11_BLEND_DESC(D3D11_DEFAULT);
	blendDesc.AlphaToCoverageEnable = FALSE;			//FALSE
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ComPtr<ID3D11BlendState> blendState = NULL;
	m_Device->CreateBlendState(&blendDesc, &blendState);
	m_DeviceContext->OMSetBlendState(blendState.Get(), blendFactor, UINT_MAX);


	// �f�v�X�X�e���V���X�e�[�g�ݒ�
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = CD3D11_DEPTH_STENCIL_DESC(D3D11_DEFAULT);
	/*depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;*/

	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStateEnable);//�[�x�L���X�e�[�g

	//depthStencilDesc.DepthEnable = FALSE;
	const D3D11_DEPTH_STENCILOP_DESC DefaultStencilOp =
	{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	depthStencilDesc.FrontFace = DefaultStencilOp;
	depthStencilDesc.BackFace = DefaultStencilOp;
	depthStencilDesc.StencilEnable = false;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_Device->CreateDepthStencilState(&depthStencilDesc, &m_DepthStateDisable);//�[�x�����X�e�[�g

	m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable.Get(), NULL);


	// �T���v���[�X�e�[�g�ݒ�
	D3D11_SAMPLER_DESC samplerDesc{};
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;	//�ٕ����t�B���^�����O
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MaxAnisotropy = 4;

	//�~�b�v�}�b�v�ݒ�
	samplerDesc.MipLODBias = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState = NULL;
	m_Device->CreateSamplerState(&samplerDesc, &samplerState);

	m_DeviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());

	// �T���v���[�X�e�[�g1�ݒ�
	D3D11_SAMPLER_DESC samplerDesc_1{};
	//samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc_1.Filter = D3D11_FILTER_ANISOTROPIC;	//�ٕ����t�B���^�����O
	samplerDesc_1.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc_1.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc_1.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc_1.MaxAnisotropy = 4;

	//�~�b�v�}�b�v�ݒ�
	samplerDesc_1.MipLODBias = 0;
	samplerDesc_1.MinLOD = 0;
	samplerDesc_1.MaxLOD = D3D11_FLOAT32_MAX;

	ComPtr<ID3D11SamplerState> samplerState_1 = NULL;
	m_Device->CreateSamplerState(&samplerDesc_1, &samplerState_1);

	m_DeviceContext->PSSetSamplers(1, 1, samplerState_1.GetAddressOf());

	// �萔�o�b�t�@����
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDesc.CPUAccessFlags = 0;
	bufferDesc.MiscFlags = 0;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.ByteWidth = sizeof(GlobalBuffer11);
	
	m_Device->CreateBuffer(&bufferDesc, NULL, &m_GlobalBuffer11);
	m_DeviceContext->VSSetConstantBuffers(0, 1, m_GlobalBuffer11.GetAddressOf());
	m_DeviceContext->PSSetConstantBuffers(0, 1, m_GlobalBuffer11.GetAddressOf());

	bufferDesc.ByteWidth = sizeof(D3DXMATRIX);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_WorldBuffer11);
	m_DeviceContext->VSSetConstantBuffers(1, 1, m_WorldBuffer11.GetAddressOf());

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_ViewBuffer11);
	m_DeviceContext->VSSetConstantBuffers(2, 1, m_ViewBuffer11.GetAddressOf());

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_ProjectionBuffer11);
	m_DeviceContext->VSSetConstantBuffers(3, 1, m_ProjectionBuffer11.GetAddressOf());

	bufferDesc.ByteWidth = sizeof(LIGHT11);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_LightBuffer11);
	m_DeviceContext->VSSetConstantBuffers(4, 1, m_LightBuffer11.GetAddressOf());
	m_DeviceContext->PSSetConstantBuffers(4, 1, m_LightBuffer11.GetAddressOf());

	/*bufferDesc.ByteWidth = sizeof(MATERIAL11);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_MaterialBuffer11);
	m_DeviceContext->VSSetConstantBuffers(5, 1, m_MaterialBuffer11.GetAddressOf());

	bufferDesc.ByteWidth = sizeof(D3DXVECTOR4);

	m_Device->CreateBuffer(&bufferDesc, NULL, &m_ParameterBuffer11);
	m_DeviceContext->VSSetConstantBuffers(6	, 1, m_ParameterBuffer11.GetAddressOf());
	m_DeviceContext->PSSetConstantBuffers(6, 1, m_ParameterBuffer11.GetAddressOf());*/

	// ���C�g������
	LIGHT11 light{};
	light.Enable = false;
	light.Direction = D3DXVECTOR4(1.0f, -1.0f, 1.0f, 0.0f);
	D3DXVec4Normalize(&light.Direction, &light.Direction);
	light.Ambient = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	SetLight(light);

	//// �}�e���A��������
	//MATERIAL11 material{};
	//material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	//material.Ambient = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	//SetMaterial(material);

	{
		//�V���h�E�o�b�t�@�쐬
		ID3D11Texture2D* DepthTexture = NULL;
		D3D11_TEXTURE2D_DESC td;
		ZeroMemory(&td, sizeof(td));
		td.Width = SwapChainDesc.BufferDesc.Width;
		td.Height = SwapChainDesc.BufferDesc.Height;
		td.MipLevels = 1;
		td.ArraySize = 1;
		td.Format = DXGI_FORMAT_R32_TYPELESS;		//32bit �̎��R�Ȍ`���̃f�[�^�Ƃ���
		td.SampleDesc = SwapChainDesc.SampleDesc;
		td.Usage = D3D11_USAGE_DEFAULT;
		td.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		td.CPUAccessFlags = 0;
		td.MiscFlags = 0;
		hr = m_Device->CreateTexture2D(&td, NULL, &DepthTexture);

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateDepthTextureFailed");
		}

		//�f�v�X�X�e���V���^�[�Q�b�g�r���[�쐬
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
		ZeroMemory(&dsvd, sizeof(dsvd));
		dsvd.Format = DXGI_FORMAT_D32_FLOAT;		//�s�N�Z���t�H�[�}�b�g��32BitFloat�^
		dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvd.Flags = 0;
		hr = m_Device->CreateDepthStencilView(DepthTexture, &dsvd, &m_ShadowDepthStencilView);

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateDepthStencilViewFailed");
		}

		//�V�F�[�_�[���\�[�X�r���[�쐬
		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = DXGI_FORMAT_R32_FLOAT;		//�s�N�Z���t�H�[�}�b�g��32BitFloat�^
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;
		hr = m_Device->CreateShaderResourceView(DepthTexture, &SRVDesc, &m_ShadowDepthShaderResourceView);

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateShaderResourceViewFailed");
		}

	}

	return hr;
}



void AppDx11::UnInit()
{
	m_Device->Release();
	m_DeviceContext->Release();
	m_SwapChain->Release();
	m_RenderTargetView->Release();
	

	m_DepthStateEnable->Release();
	m_DepthStateDisable->Release();
	m_DepthStencilView->Release();
	
	m_GlobalBuffer11->Release();
	m_WorldBuffer11->Release();
	m_ViewBuffer11->Release();
	m_ProjectionBuffer11->Release();
	m_LightBuffer11->Release();
	//m_MaterialBuffer11->Release();
}




void AppDx11::Begin()
{
	m_DeviceContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());

	float clearColor[4] = { ImGui11::ClearColor.x * ImGui11::ClearColor.w, ImGui11::ClearColor.y * ImGui11::ClearColor.w, ImGui11::ClearColor.z * ImGui11::ClearColor.w, ImGui11::ClearColor.w };
	m_DeviceContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
	m_DeviceContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}



void AppDx11::End()
{
	m_SwapChain->Present(1, 0);
}




void AppDx11::SetDepthEnable(bool Enable)
{
	if (Enable)
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateEnable.Get(), NULL);
	else
		m_DeviceContext->OMSetDepthStencilState(m_DepthStateDisable.Get(), NULL);

}

void AppDx11::SetWorldViewProjection2D()
{
	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	D3DXMatrixTranspose(&world, &world);

	m_DeviceContext->UpdateSubresource(m_WorldBuffer11.Get(), 0, NULL, &world, 0, 0);

	D3DXMATRIX view;
	D3DXMatrixIdentity(&view);
	D3DXMatrixTranspose(&view, &view);
	m_DeviceContext->UpdateSubresource(m_ViewBuffer11.Get(), 0, NULL, &view, 0, 0);

	D3DXMATRIX projection;
	D3DXMatrixOrthoOffCenterLH(&projection, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
	D3DXMatrixTranspose(&projection, &projection);
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer11.Get(), 0, NULL, &projection, 0, 0);

}

void AppDx11::SetGlobal(GlobalBuffer11 global)
{
	m_DeviceContext->UpdateSubresource(m_GlobalBuffer11.Get(), 0, NULL, &global, 0, 0);
}

void AppDx11::SetWorldMatrix(D3DXMATRIX* WorldMatrix)
{
	D3DXMATRIX world;
	D3DXMatrixTranspose(&world, WorldMatrix);
	m_DeviceContext->UpdateSubresource(m_WorldBuffer11.Get(), 0, NULL, &world, 0, 0);
}

void AppDx11::SetViewMatrix(D3DXMATRIX* ViewMatrix)
{
	D3DXMATRIX view;
	D3DXMatrixTranspose(&view, ViewMatrix);
	m_DeviceContext->UpdateSubresource(m_ViewBuffer11.Get(), 0, NULL, &view, 0, 0);
}

void AppDx11::SetProjectionMatrix(D3DXMATRIX* ProjectionMatrix)
{
	D3DXMATRIX projection;
	D3DXMatrixTranspose(&projection, ProjectionMatrix);
	m_DeviceContext->UpdateSubresource(m_ProjectionBuffer11.Get(), 0, NULL, &projection, 0, 0);
}

//void AppDx11::SetMaterial(MATERIAL11 Material)
//{
//	m_DeviceContext->UpdateSubresource(m_MaterialBuffer11.Get(), 0, NULL, &Material, 0, 0);
//}

void AppDx11::SetLight(LIGHT11 Light)
{
	//�V�F�[�_�[���̓s����ōs���]�u���Ă���
	D3DXMatrixTranspose(&Light.ViewMatrix, &Light.ViewMatrix);
	D3DXMatrixTranspose(&Light.ProjectionMatrix, &Light.ProjectionMatrix);

	m_DeviceContext->UpdateSubresource(m_LightBuffer11.Get(), 0, NULL, &Light, 0, 0);
}

//void AppDx11::SetParameter(D3DXVECTOR4 Parameter)
//{
//	m_DeviceContext->UpdateSubresource(m_ParameterBuffer11.Get(), 0, NULL, &Parameter, 0, 0);
//}

void AppDx11::CreateVertexShader(ID3D11VertexShader** VertexShader, ID3D11InputLayout** VertexLayout, const char* FileName)
{

	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreateVertexShader(buffer, fsize, NULL, VertexShader);


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 4 * 3, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 4 * 6, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 4 * 10, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT numElements = ARRAYSIZE(layout);

	m_Device->CreateInputLayout(layout,
		numElements,
		buffer,
		fsize,
		VertexLayout);

	delete[] buffer;
}



void AppDx11::CreatePixelShader(ID3D11PixelShader** PixelShader, const char* FileName)
{
	FILE* file;
	long int fsize;

	file = fopen(FileName, "rb");
	fsize = _filelength(_fileno(file));
	unsigned char* buffer = new unsigned char[fsize];
	fread(buffer, fsize, 1, file);
	fclose(file);

	m_Device->CreatePixelShader(buffer, fsize, NULL, PixelShader);

	delete[] buffer;
}