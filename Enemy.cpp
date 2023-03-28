#include "GameObject.h"
#include "AnimationModel11.h"
#include "AnimationModel12.h"
#include "RootSignature.h"
#include "Enemy.h"
#include "camera.h"
#include "GameManager.h"

AnimationModel11* Enemy::m_Model11 = NULL;
AnimationModel12* Enemy::m_Model12 = NULL;
RootSignature* Enemy::m_RootSignature = NULL;

//RootSignature* Enemy::m_RootSignatureShadow = NULL;

void Enemy::Init(UINT version)
{
	if (version == 0)
	{
		GetDX11Renderer->CreateVertexShader(m_VertexShader11.GetAddressOf(), m_VertexLayout11.GetAddressOf(),
			"shader\\pixelLightingVS.cso");
		GetDX11Renderer->CreatePixelShader(m_PixelShader11.GetAddressOf(), "shader\\pixelLightingPS.cso");

		//�e�p
		{
			GetDX11Renderer->CreateVertexShader(&m_VertexShaderShadow11, m_VertexLayout11.GetAddressOf(),
				"shader\\ShadowVS.cso");
			GetDX11Renderer->CreatePixelShader(&m_PixelShaderShadow11, "shader\\ShadowPS.cso");
		}

		m_Position11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Scale11 = D3DXVECTOR3(0.01f, 0.01f, 0.01f);

		speed11 = 0.05f;
		m_BlendRate11 = 0.0f;
		m_AnimationSpeed = 0.03f;

		m_AnimationNormal11 = "Stay";
		m_AnimationAction11 = "Stay";
	}
	else if (version == 1)
	{
		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\pixelLightingVS.hlsl");
		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\pixelLightingPS.hlsl");

		//�p�C�v���C���X�e�[�g
		m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature());

		////�e�p
		//{
		//	m_VertexShaderShadow12 = GetDX12Renderer->CreateVertexShader(L"shader\\ShadowVS.hlsl");
		//	m_PixelShaderShadow12 = GetDX12Renderer->CreatePixelShader(L"shader\\ShadowPS.hlsl");

		//	m_PipelineStateShadow12 = GetDX12Renderer->CreatePipelineState(m_VertexShaderShadow12, m_PixelShaderShadow12, m_RootSignatureShadow->GetRootSignature());
		//}

		m_AnimationNormal12 = "Stay";
		m_AnimationAction12 = "Stay";

		m_Position12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Scale12 = XMFLOAT3(0.01f, 0.01f, 0.01f);

		speed12 = 0.05f;
		m_BlendRate12 = 0.0f;
		m_AnimationSpeed = 0.03f;
	}
}

void Enemy::UnInit(UINT version)
{
	if (version == 0)
	{
		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();
		m_VertexShaderShadow11->Release();
		m_PixelShaderShadow11->Release();

		m_Model11->UnLoad();
	}
	else if (version == 1)
	{
		m_RootSignature->UnInit();
		m_PipelineState12->Release();
		//m_RootSignatureShadow->UnInit();
		//m_PipelineStateShadow12->Release();

		m_Model12->UnLoad();
	}
}

void Enemy::Update(UINT version)
{
	if (version == 0)
	{

	}
	else if (version == 1)
	{
		bool AnimationReturn = true;

		//�d��
		if (m_Position12.y > 0.0f)
			m_Velocity12.y -= 0.01f;

		////�A�j���V�����߂鎞
		//if (AnimationReturn)
		//	m_BlendRate12 -= m_AnimationSpeed;

		//�A�j���[�V�������E
		if (m_BlendRate12 > 1.0f)
		{
			m_BlendRate12 = 1.0f;
		}
		else if (m_BlendRate12 < 0.0f)
		{
			m_BlendRate12 = 0.0f;
			m_AnimationAction12 = "Stay";
		}

		m_Frame12++;
	}
}

void Enemy::Draw(UINT version)
{
	if (version == 0)
	{
		auto m_DeviceContext = GetDX11Renderer->GetDeviceContext().Get();

		m_Model11->Update(m_AnimationNormal11.c_str(), m_AnimationAction11.c_str(), m_BlendRate11, m_Frame11);

		//���̓��C�A�E�g�ݒ�
		m_DeviceContext->IASetInputLayout(m_VertexLayout11.Get());

		//�V�F�[�_�[�ݒ�
		m_DeviceContext->VSSetShader(m_VertexShader11.Get(), NULL, 0);
		m_DeviceContext->PSSetShader(m_PixelShader11.Get(), NULL, 0);

		//���[���h�}�g���N�X�ݒ�
		D3DXMATRIX world, scale, rot, trans;
		D3DXMatrixScaling(&scale, m_Scale11.x, m_Scale11.y, m_Scale11.z);
		D3DXMatrixRotationYawPitchRoll(&rot, m_Rotation11.y, m_Rotation11.x, m_Rotation11.z);

		D3DXMatrixTranslation(&trans, m_Position11.x, m_Position11.y, m_Position11.z);
		world = scale * rot * trans;
		GetDX11Renderer->SetWorldMatrix(&world);

		m_Model11->Draw();

		//�e�p
		{
			//���̓��C�A�E�g�ݒ�
			m_DeviceContext->IASetInputLayout(m_VertexLayout11.Get());

			//�V�F�[�_�[�ݒ�
			m_DeviceContext->VSSetShader(m_VertexShaderShadow11, NULL, 0);
			m_DeviceContext->PSSetShader(m_PixelShaderShadow11, NULL, 0);

			D3DXMATRIX shadow;

			// �e�s�񐶐�
			D3DXMatrixIdentity(&shadow);

			// ���ʂ̖@���x�N�g��
			// ��4�����͕��ʂƌ��_�̋���(���_����Ȃ�΁|�A���Ȃ�΁{�̕�����t��)
			// ������������ɂ��炵�e�𕂂����邱�Ƃ�Z�t�@�C�e�B���O�΍���s��
			D3DXVECTOR4 normal = D3DXVECTOR4(0.0f, 5.0f, 0.0f, -0.1f);
			// �����ւ̃x�N�g��(���s����/�_�����ŕς��Ă݂悤)
			D3DXVECTOR4 light = D3DXVECTOR4(-1.0f, 1.0f, -1.0f, 0.0f);

			float dot =
				normal.x * light.x +
				normal.y * light.y +
				normal.z * light.z +
				normal.w * light.w;

			shadow._11 = -light.x * normal.x + dot;
			shadow._21 = -light.x * normal.y;
			shadow._31 = -light.x * normal.z;
			shadow._41 = -light.x * normal.w;

			shadow._12 = -light.y * normal.x;
			shadow._22 = -light.y * normal.y + dot;
			shadow._32 = -light.y * normal.z;
			shadow._42 = -light.y * normal.w;

			shadow._13 = -light.z * normal.x;
			shadow._23 = -light.z * normal.y;
			shadow._33 = -light.z * normal.z + dot;
			shadow._43 = -light.z * normal.w;

			shadow._14 = -light.w * normal.x;
			shadow._24 = -light.w * normal.y;
			shadow._34 = -light.w * normal.z;
			shadow._44 = -light.w * normal.w + dot;

			D3DXMatrixMultiply(&world, &world, &shadow);
			GetDX11Renderer->SetWorldMatrix(&world);

			m_Model11->Draw();
		}
	}
	else if (version == 1)
	{
		auto m_CommandList = GetDX12Renderer->GetCommandList();

		Scene* scene = GameManager::GetScene();
		Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 1);

		m_Model12->Update(m_AnimationNormal12.c_str(), m_AnimationAction12.c_str(), m_BlendRate12, m_Frame12);

		//���[�g�V�O�l�`�����Z�b�g
		m_RootSignature->Draw();

		m_CommandList->SetPipelineState(m_PipelineState12.Get());

		XMMATRIX world, scale, rot, trans;

		scale = XMMatrixScaling(m_Scale12.x, m_Scale12.y, m_Scale12.z);
		rot = XMMatrixRotationRollPitchYaw(m_Rotation12.x, m_Rotation12.y, m_Rotation12.z);
		trans = XMMatrixTranslation(m_Position12.x, m_Position12.y, m_Position12.z);

		world = scale * rot * trans;

		//�R���X�^���g�o�b�t�@�̓��e���X�V
		{
			GlobalBuffer12 gb;
			gb.g_CameraPosition = XMFLOAT4(camera->GetPosition12().x, camera->GetPosition12().y, camera->GetPosition12().z, 1.0f);

			GetDX12Renderer->SetGlobalBuffer(&gb, m_Model12->GetGlobalBuffer());

			XMMATRIX view, projection;

			view = camera->GetViewMatrix12();
			projection = camera->GetProjMatrix12();

			GetDX12Renderer->SetWorldMatrix(world, m_Model12->GetWorldBuffer());
			GetDX12Renderer->SetViewMatrix(view, m_Model12->GetViewBuffer());
			GetDX12Renderer->SetProjectMatrix(projection, m_Model12->GetProjectionBuffer());

			//���C�g�X�V
			LIGHT12 light;
			light.Enable = true;
			XMFLOAT4 normalize = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
			light.Direction = XMVector4Normalize(XMLoadFloat4(&normalize));
			light.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			GetDX12Renderer->SetLight(&light, m_Model12->GetLightBuffer());
		}

		m_Model12->Draw();

		////�e�p
		//{
		//	//���[�g�V�O�l�`�����Z�b�g
		//	m_RootSignatureShadow->Draw();

		//	m_CommandList->SetPipelineState(m_PipelineStateShadow12.Get());

		//	world = scale * rot * trans;

		//	m_Model12->DrawShadow(world);
		//}
	}
}

void Enemy::Load(UINT version)
{
	if (version == 0)
	{
		m_Model11 = new AnimationModel11();
		m_Model11->Load("Asset\\model\\PlayerIdle2.fbx");
		m_Model11->LoadAnimation("Asset\\model\\PlayerRun2.fbx", "Run");
		m_Model11->LoadAnimation("Asset\\model\\PlayerIdle2.fbx", "Stay");
	}
	else if (version == 1)
	{
		m_Model12 = new AnimationModel12(2560, 2560, DDS_FORMAT);
		m_Model12->Load("Asset\\model\\PlayerIdle.fbx", false);
		m_Model12->LoadAnimation("Asset\\model\\PlayerIdle.fbx", "Stay");
		m_Model12->LoadAnimation("Asset\\model\\PlayerRun.fbx", "Run");
		//m_Model12->LoadAnimation("Asset\\model\\PlayerSlash.fbx", "Slash");

		m_RootSignature = new RootSignature;
		m_RootSignature->InitNormal();

		////�e�p
		//{
		//	//constantBuffer��
		//	CD3DX12_DESCRIPTOR_RANGE1 ranges[4];
		//	CD3DX12_ROOT_PARAMETER1 RootParameters[1];
		//	ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//	ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//	ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//	ranges[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
		//	//CBV Table
		//	RootParameters[0].InitAsDescriptorTable(4, &ranges[0], D3D12_SHADER_VISIBILITY_ALL);

		//	//m_RootSignatureShadow = GetDX12Renderer->CreateRootSignature(RootParameters, _countof(RootParameters));
		//	m_RootSignatureShadow = new RootSignature;
		//	m_RootSignatureShadow->InitCustomize(_countof(RootParameters), RootParameters, NULL);
		//}
	}
}
