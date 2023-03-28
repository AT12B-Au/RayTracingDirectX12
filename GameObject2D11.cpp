#include "AppDx11.h"
#include "main.h"
#include "RendererManager.h"
#include "GameObject2D11.h"

void GameObject2D11::Init(VERTEX_3D11* Vertex11,const char* TextureName,const char* VertexShaderName,
	const char* PixelShaderName)
{

	auto m_Device = GetDX11Renderer->GetDevice().Get();
	HRESULT hr;
	//���_���W
	{
		//���_�o�b�t�@����
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.ByteWidth = sizeof(VERTEX_3D11) * 4;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA	sd{};
		sd.pSysMem = &Vertex11;

		hr = m_Device->CreateBuffer(&bd, &sd, &m_VertexBuffer11);
		if (FAILED(hr))
		{
			throw std::runtime_error("CreateVertexBufferFailed");
		}
	}

	//�e�N�X�`��
	if (TextureName != NULL)
	{
		//�e�N�X�`���ǂݍ���
		D3DX11CreateShaderResourceViewFromFile(m_Device,
			TextureName,
			NULL,
			NULL,
			&m_Texture11,
			NULL);
		assert(m_Texture11);
	}
	else
		HaveTexture = false;

	//�V�F�[�_�[
	{
		GetDX11Renderer->CreateVertexShader(&m_VertexShader11, &m_VertexLayout11, VertexShaderName);
		GetDX11Renderer->CreatePixelShader(&m_PixelShader11, PixelShaderName);
	}
}

void GameObject2D11::InitNormal(const char* TextureName)
{
	D3DX11CreateShaderResourceViewFromFile(GetDX11Renderer->GetDevice().Get(),
		TextureName,
		NULL,
		NULL,
		&m_TextureNormal11,
		NULL);

	assert(m_TextureNormal11);
}

void GameObject2D11::UnInit()
{
	m_VertexBuffer11->Release();

	if (HaveTexture != false)
		m_Texture11->Release();

	if (m_TextureNormal11 != NULL)
		m_TextureNormal11->Release();

	m_VertexLayout11->Release();
	m_VertexShader11->Release();
	m_PixelShader11->Release();
}

void GameObject2D11::Draw()
{
	//���̓��C�A�E�g�ݒ�
	GetDX11Renderer->GetDeviceContext()->IASetInputLayout(m_VertexLayout11.Get());

	//�V�F�[�_�[�ݒ�
	GetDX11Renderer->GetDeviceContext()->VSSetShader(m_VertexShader11.Get(), NULL, 0);
	GetDX11Renderer->GetDeviceContext()->PSSetShader(m_PixelShader11.Get(), NULL, 0);

	//�}�g���N�X�ݒ�
	GetDX11Renderer->SetWorldViewProjection2D();

	//���_�o�b�t�@�ݒ�
	UINT stride = sizeof(VERTEX_3D11);
	UINT offset = 0;
	GetDX11Renderer->GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer11, &stride, &offset);

	//// �}�e���A���ݒ�
	//MATERIAL11 material;
	//ZeroMemory(&material, sizeof(material));
	//material.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	//GetDX11Renderer->SetMaterial(material);

	//�e�N�X�`���ݒ�
	if (HaveTexture != false)
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture11);

	//�v���~�e�B�u�|���W�ݒ�
	GetDX11Renderer->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//�|���S���`��
	GetDX11Renderer->GetDeviceContext()->Draw(4, 0);
}

void GameObject2D11::SetNormal()
{
	//�m�[�}���}�b�v�ݒ�
	if (m_TextureNormal11 != NULL)
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(1, 1, &m_TextureNormal11);
}

void GameObject2D11::UpdateVertex(VERTEX_3D11* Vertex11)
{
	auto m_DeviceContext = GetDX11Renderer->GetDeviceContext().Get();

	//���_�f�[�^��������
	D3D11_MAPPED_SUBRESOURCE msr;

	HRESULT hr = m_DeviceContext->Map(m_VertexBuffer11.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
	if (FAILED(hr))
	{
		throw std::runtime_error("UpdateVertexFailed");
	}
	VERTEX_3D11* vertex = (VERTEX_3D11*)msr.pData;

	memcpy(msr.pData, &Vertex11, sizeof(Vertex11));

	m_DeviceContext->Unmap(m_VertexBuffer11.Get(), 0);
}
