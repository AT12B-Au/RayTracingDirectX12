#include "AppDx11.h"
#include "AppDx12.h"
#include "RendererManager.h"
#include "GameObject.h"
#include "RootSignature.h"
#include "Transition.h"
#include "main.h"

void Transition::Init(UINT version)
{
	if (version == 0)
	{
		auto m_Device = GetDX11Renderer->GetDevice();

		VERTEX_3D11 m_Vertex11[4];

		m_Vertex11[0].Position = D3DXVECTOR3(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 200, 0.0f);
		m_Vertex11[0].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[0].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[0].TexCoord = D3DXVECTOR2(0.0f, 0.0f);

		m_Vertex11[1].Position = D3DXVECTOR3(SCREEN_WIDTH, SCREEN_HEIGHT - 200, 0.0f);
		m_Vertex11[1].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[1].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[1].TexCoord = D3DXVECTOR2(1.0f, 0.0f);

		m_Vertex11[2].Position = D3DXVECTOR3(SCREEN_WIDTH - 200, SCREEN_HEIGHT, 0.0f);
		m_Vertex11[2].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[2].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[2].TexCoord = D3DXVECTOR2(0.0f, 1.0f);

		m_Vertex11[3].Position = D3DXVECTOR3(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
		m_Vertex11[3].Normal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Vertex11[3].Diffuse = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex11[3].TexCoord = D3DXVECTOR2(1.0f, 1.0f);

		// ���_�o�b�t�@����
		{
			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(VERTEX_3D11) * 4;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = &m_Vertex11;

			GetDX11Renderer->GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer11);
		}

		//�e�N�X�`���ǂݍ���
		{
			D3DX11CreateShaderResourceViewFromFile(m_Device.Get(),
				"Asset\\texture\\Loading.dds",
				NULL,
				NULL,
				&m_Texture11,
				NULL);
			assert(m_Texture11);
		}

		//�V�F�[�_�[
		{
			GetDX11Renderer->CreateVertexShader(&m_VertexShader11, &m_VertexLayout11,
				"shader\\PhongVS.cso");
			GetDX11Renderer->CreatePixelShader(&m_PixelShader11, "shader\\PhongPS.cso");
		}

		m_Position11 = D3DXVECTOR3(SCREEN_WIDTH - 100,
			SCREEN_HEIGHT - 100,
			0.0f);
		m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}
	else if (version == 1)
	{
		auto m_Device = GetDX12Renderer->GetDevice();
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_CommandQueue = GetDX12Renderer->GetCommandQueue();
		auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();

		m_Vertex12[0].Position = XMFLOAT3(SCREEN_WIDTH - 200, SCREEN_HEIGHT - 200, 0.0f);
		m_Vertex12[0].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[0].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[0].TexCoord = XMFLOAT2(0.0f, 0.0f);

		m_Vertex12[1].Position = XMFLOAT3(SCREEN_WIDTH, SCREEN_HEIGHT - 200, 0.0f);
		m_Vertex12[1].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[1].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[1].TexCoord = XMFLOAT2(1.0f, 0.0f);

		m_Vertex12[2].Position = XMFLOAT3(SCREEN_WIDTH - 200, SCREEN_HEIGHT, 0.0f);
		m_Vertex12[2].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[2].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[2].TexCoord = XMFLOAT2(0.0f, 1.0f);

		m_Vertex12[3].Position = XMFLOAT3(SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f);
		m_Vertex12[3].Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Vertex12[3].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		m_Vertex12[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

		//�R�}���h���X�g�ɏ������ޑO�ɂ̓R�}���h�A���P�[�^�[�����Z�b�g����
		m_CommandAllocator->Reset();
		//�R�}���h���X�g�����Z�b�g����
		m_CommandList->Reset(m_CommandAllocator.Get(), NULL);

		//�o�[�e�b�N�X�o�b�t�@�쐬
		{
			const UINT VertexBufferSize = sizeof(VERTEX_3D12) * 4;
			m_VertexBuffer12 = GetDX12Renderer->CreateVertexBuffer(VertexBufferSize);
			m_VertexBufferUpLoad12 = GetDX12Renderer->CreateVertexBufferUpLoad(VertexBufferSize);
			m_VertexBufferView12 = GetDX12Renderer->CreateVertexBufferView(m_VertexBufferUpLoad12, VertexBufferSize);

			//���_�o�b�t�@�̍X�V
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = &m_Vertex12[0];
			vertexData.RowPitch = VertexBufferSize;
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12.Get(), m_VertexBufferUpLoad12.Get(), 0, 0, 1, &vertexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//�C���f�b�N�X�o�b�t�@�쐬
		{
			DWORD index[6] =
			{
				0,1,2,
				1,3,2
			};

			UINT m_IndexBufferSize = sizeof(index);
			m_IndexBuffer12 = GetDX12Renderer->CreateIndexBuffer(m_IndexBufferSize);
			m_IndexBufferUpLoad12 = GetDX12Renderer->CreateIndexBufferUpLoad(m_IndexBufferSize);
			m_IndexBufferView12 = GetDX12Renderer->CreateIndexBufferView(m_IndexBufferUpLoad12, m_IndexBufferSize);

			D3D12_SUBRESOURCE_DATA IndexData = {};
			IndexData.pData = &index[0];
			IndexData.RowPitch = m_IndexBufferSize;
			IndexData.SlicePitch = IndexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_IndexBuffer12.Get(), m_IndexBufferUpLoad12.Get(), 0, 0, 1, &IndexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_IndexBuffer12.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//�q�[�v�̍쐬 && �R���X�^���g�o�b�t�@�̍쐬
		{
			UINT CBSize;
			CBSize = (sizeof(GlobalBuffer12) + 255) & ~255;

			m_DescHeap = GetDX12Renderer->CreateConstantBufferViewHeap(5);

			m_GlobalBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);

			m_CBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_GlobalBuffer12, m_DescHeap, 0, CBSize);

			CBSize = (sizeof(XMMATRIX) + 255) & ~255;
			m_WorldBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_WBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_WorldBuffer12, m_DescHeap, 1, CBSize);

			m_ViewBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_VBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ViewBuffer12, m_DescHeap, 2, CBSize);

			m_ProjectionBuffer12 = GetDX12Renderer->CreateConstantBuffer(CBSize);
			m_PBVDesc12 = GetDX12Renderer->CreateConstantBufferView(m_ProjectionBuffer12, m_DescHeap, 3, CBSize);
		}

		//�e�N�X�`�����[�h
		{
			m_Texture12 = GetDX12Renderer->CreateTexture(DDS_FORMAT,SCREEN_WIDTH, SCREEN_HEIGHT);

			m_TextureUpLoad12 = GetDX12Renderer->CreateTextureUpLoad(m_Texture12);

			//�t�@�C������e�N�Z����T�C�Y����ǂݏo��(���̏����͊��S�Ƀ��[�U�[����j
			std::unique_ptr<uint8_t[]> DecodedData;
			std::vector<D3D12_SUBRESOURCE_DATA> Subres;

			HRESULT hr = LoadDDSTextureFromFile(m_Device.Get(), L"Asset\\texture\\Loading.dds", &m_Texture12, DecodedData, Subres);
			if (FAILED(hr))
			{
				throw std::runtime_error("LoadTexture2DFailed");
			}

			GetDX12Renderer->CreateTextureView(m_Texture12.Get(), m_DescHeap, DDS_FORMAT, 4);

			const UINT SubresSize = static_cast<UINT>(Subres.size());

			//�ǂ݂������e�N�Z���f�[�^���󔒃e�N�X�`���[�ɗ������݁A�e�N�X�`���[�Ƃ��Ċ���������
			UpdateSubresources(m_CommandList.Get(), m_Texture12.Get(), m_TextureUpLoad12.Get(), 0, 0, SubresSize, &Subres[0]);
			CD3DX12_RESOURCE_BARRIER tran = CD3DX12_RESOURCE_BARRIER::Transition(m_Texture12.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			m_CommandList->ResourceBarrier(1, &tran);

			//�����ň�U�R�}���h�����B�e�N�X�`���[�̓]�����J�n���邽��
			m_CommandList->Close();

			//�R�}���h���X�g�̎��s�@
			ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
			m_CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

			//�����i�ҋ@�j�@�e�N�X�`���[�̓]�����I���܂őҋ@
			GetDX12Renderer->WaitGpu();

			m_TextureUpLoad12->Release();
		}

		//���[�g�V�O�l�`��
		{
			CD3DX12_DESCRIPTOR_RANGE1 ranges[5];
			CD3DX12_ROOT_PARAMETER1 RootParameters[1 + 1];
			ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			ranges[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			//CBV Table
			RootParameters[0].InitAsDescriptorTable(4, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			//SRV Table
			RootParameters[1].InitAsDescriptorTable(1, &ranges[4], D3D12_SHADER_VISIBILITY_ALL);

			m_RootSignature = new RootSignature;
			m_RootSignature->InitCustomize(_countof(RootParameters), RootParameters, NULL);
		}

		//�V�F�[�_�[
		{
			m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\PhongVS.hlsl");
			m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\PhongPS.hlsl");

			m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature());
		}

		m_Position12 = XMFLOAT3(SCREEN_WIDTH - 100,
			SCREEN_HEIGHT - 100,
			0.0f);
		m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
}

void Transition::UnInit(UINT version)
{
	if (version == 0)
	{
		m_VertexBuffer11->Release();
		m_Texture11->Release();

		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();
	}
	else if (version == 1)
	{
		m_VertexBuffer12->Release();
		m_VertexBufferUpLoad12->Release();

		m_IndexBuffer12->Release();
		m_IndexBufferUpLoad12->Release();

		m_GlobalBuffer12->Release();
		m_WorldBuffer12->Release();
		m_ViewBuffer12->Release();
		m_ProjectionBuffer12->Release();

		m_Texture12->Release();

		m_DescHeap->Release();

		m_PipelineState12->Release();
		m_RootSignature->UnInit();
	}
}

void Transition::Update(UINT version)
{
	if (version == 0)
	{
		m_Rotation11.z += 0.05f;
	}
	else if (version == 1)
	{
		m_Rotation12.z += 0.05f;
	}
}

void Transition::Draw(UINT version)
{
	if (version == 0)
	{
		auto m_DeviceContext = GetDX11Renderer->GetDeviceContext();

		//���̓��C�A�E�g�ݒ�
		m_DeviceContext->IASetInputLayout(m_VertexLayout11.Get());

		//�V�F�[�_�[�ݒ�
		m_DeviceContext->VSSetShader(m_VertexShader11.Get(), NULL, 0);
		m_DeviceContext->PSSetShader(m_PixelShader11.Get(), NULL, 0);

		MATERIAL11 material;
		material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, alpha);

		GlobalBuffer11 gb;
		gb.material = material;
		GetDX11Renderer->SetGlobal(gb);

		//�}�g���N�X�ݒ�
		GetDX11Renderer->SetWorldViewProjection2D();

		D3DXMATRIX world, rot, trans_1, trans_2;

		D3DXMatrixRotationYawPitchRoll(&rot, m_Rotation11.y, m_Rotation11.x, m_Rotation11.z);
		D3DXMatrixTranslation(&trans_1, -m_Position11.x, -m_Position11.y, -m_Position11.z);
		D3DXMatrixTranslation(&trans_2, m_Position11.x, m_Position11.y, m_Position11.z);

		world = trans_1 * rot * trans_2;

		GetDX11Renderer->SetWorldMatrix(&world);

		//���_�o�b�t�@�ݒ�
		UINT stride = sizeof(VERTEX_3D11);
		UINT offset = 0;
		m_DeviceContext->IASetVertexBuffers(0, 1, m_VertexBuffer11.GetAddressOf(), &stride, &offset);

		//�e�N�X�`���ݒ�
		m_DeviceContext->PSSetShaderResources(0, 1, m_Texture11.GetAddressOf());

		//�v���~�e�B�u�|���W�ݒ�
		m_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//�|���S���`��
		m_DeviceContext->Draw(4, 0);
	}
	else if (version == 1)
	{
		auto m_Device = GetDX12Renderer->GetDevice();
		auto m_CommandList = GetDX12Renderer->GetCommandList();

		//���_�o�b�t�@�X�V
		{
			D3D12_SUBRESOURCE_DATA vertexData = {};
			vertexData.pData = &m_Vertex12[0];
			vertexData.RowPitch = sizeof(VERTEX_3D12) * 4;
			vertexData.SlicePitch = vertexData.RowPitch;
			UpdateSubresources<1>(m_CommandList.Get(), m_VertexBuffer12.Get(), m_VertexBufferUpLoad12.Get(), 0, 0, 1, &vertexData);
			CD3DX12_RESOURCE_BARRIER trans = CD3DX12_RESOURCE_BARRIER::Transition(m_VertexBuffer12.Get(),
				D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
			m_CommandList->ResourceBarrier(1, &trans);
		}

		//���[�g�V�O�l�`�����Z�b�g
		m_RootSignature->Draw();

		m_CommandList->SetPipelineState(m_PipelineState12.Get());

		//�q�[�v���Z�b�g
		ID3D12DescriptorHeap* ppHeaps[] = { m_DescHeap.Get() };
		m_CommandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

		//�|���S���g�|���W�[�̎w��
		m_CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

		//�o�[�e�b�N�X�o�b�t�@���Z�b�g
		m_CommandList->IASetVertexBuffers(0, 1, &m_VertexBufferView12);

		//�C���f�b�N�X�o�b�t�@���Z�b�g
		m_CommandList->IASetIndexBuffer(&m_IndexBufferView12);

		

		MATERIAL12 material;
		material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, alpha);

		GlobalBuffer12 gb;
		gb.material = material;

		GetDX12Renderer->SetGlobalBuffer(&gb, m_GlobalBuffer12);

		XMMATRIX world, rot, trans_1, trans_2;

		rot = XMMatrixRotationRollPitchYaw(m_Rotation12.x, m_Rotation12.y, m_Rotation12.z);
		trans_1 = XMMatrixTranslation(-m_Position12.x, -m_Position12.y, -m_Position12.z);
		trans_2 = XMMatrixTranslation(m_Position12.x, m_Position12.y, m_Position12.z);

		world = trans_1 * rot * trans_2;

		GetDX12Renderer->SetWorldMatrix(world,m_WorldBuffer12);

		XMMATRIX view = GetDX12Renderer->SetWorldViewProjection2D(1);
		GetDX12Renderer->SetViewMatrix(view, m_ViewBuffer12);
		XMMATRIX proj = GetDX12Renderer->SetWorldViewProjection2D(2);
		GetDX12Renderer->SetProjectMatrix(proj, m_ProjectionBuffer12);

		//�n���h���Z�b�g
		{
			////CBV�Z�b�g
			D3D12_GPU_DESCRIPTOR_HANDLE CbvSrvUavHandle = m_DescHeap->GetGPUDescriptorHandleForHeapStart();
			m_CommandList->SetGraphicsRootDescriptorTable(0, CbvSrvUavHandle);

			CbvSrvUavHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(
				m_DescHeap->GetGPUDescriptorHandleForHeapStart(),
				4, m_Device->GetDescriptorHandleIncrementSize(
					D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
			m_CommandList->SetGraphicsRootDescriptorTable(1, CbvSrvUavHandle);
		}

		m_CommandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
	}
}
