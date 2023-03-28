#include <Windows.h>
#include "AppDx11.h"
#include "AppDx12.h"
#include "RendererManager.h"
#include "main.h"
#include "GameManager.h"
#include "ImGui11.h"
#include "ImGui12.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "camera.h"
#include "GameObject2D12.h"
#include "Lake.h"
#include "input.h"
#include "Player.h"

AppDx11* RendererManager::m_mainDx11 = NULL;
AppDx12* RendererManager::m_mainDx12 = NULL;


void RendererManager::Init(UINT version)
{
	if (version == IDYES)			//DX11�̏�����
	{
		delete m_mainDx12;
		m_mainDx11 = new AppDx11;
		HRESULT hr = m_mainDx11->Init();
		//DX11�N���ł��Ȃ���
		if (FAILED(hr))
		{
			MessageBoxA(0, "DirectX11�N���ł��܂���", "�N���G���[", MB_ICONERROR);
			exit(0);
		}
		else
			GameManager::Init(0);
	}
	else if (version == IDNO)		//DX12�̏�����
	{
		delete m_mainDx11;
		m_mainDx12 = new AppDx12;
		HRESULT hr = m_mainDx12->Init();
		//DX12�N���ł��Ȃ���
		if (FAILED(hr))
		{
			MessageBoxA(0, "DirectX12�N���ł��܂���", "�N���G���[", MB_ICONERROR);
			exit(0);
		}
		else
			GameManager::Init(1);
	}
	else
	{
		MessageBoxA(0, "Manager::Init Error", "Error", MB_ICONERROR);
	}
}

void RendererManager::UnInit(UINT version)
{
	
	if (version == IDYES)			//DX11�I�����鎞
	{
		auto m_Device = GetDX11Renderer->GetDevice().Get();

	#if defined(_DEBUG)

		HRESULT hr;

		//�f�o�b�N���C�A�E�g
		ComPtr<ID3D11Debug> m_DebugController;

		//�f�o�b�N���C�A�E�g
		hr = m_Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(m_DebugController.GetAddressOf()));

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateDebugController11Failed");
		}
	#endif
		
		GameManager::UnInit(0);
		ImGui_ImplDX11_Shutdown();

		m_mainDx11->UnInit();
	#if defined(_DEBUG)
		hr = m_DebugController->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL | D3D11_RLDO_IGNORE_INTERNAL);
	#endif
	
	}
	else if (version == IDNO)		//DX12�I�����鎞
	{
		
	#if defined(_DEBUG)	
			ComPtr<ID3D12DebugDevice> pDebugDevice;
			HRESULT hr;
	
			hr = GetDX12Renderer->GetDevice()->QueryInterface(pDebugDevice.GetAddressOf());
			GetDX12Renderer->GetDebug12()->Release();
			
	#endif

		GameManager::UnInit(1);
		ImGui_ImplDX12_Shutdown();
		m_mainDx12->WaitGpu();

		m_mainDx12->UnInit();
	#if defined(_DEBUG)
		hr = pDebugDevice->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
	#endif
	}
	else
	{
		MessageBoxA(0, "Manager::UnInit Error", "Error", MB_ICONERROR);
	}
}

void RendererManager::Update(UINT version)
{
	if (version == IDYES)
	{
		GameManager::Update(0);

		ImGui11::Update();
	}
	else if (version == IDNO)
	{
		GetDX12Renderer->Update();

		GameManager::Update(1);

		ImGui12::Update();
	}

}

void RendererManager::Draw(UINT version)
{
	if (version == IDYES)
	{
		//���C�g�̏����\���̂փZ�b�g
		LIGHT11 light;
		light.Enable = true;
		light.Direction = D3DXVECTOR4(0.0f, -1.0f, 0.0f, 0.0f);//���̐i�ޕ���
		D3DXVec4Normalize(&light.Direction, &light.Direction);
		light.Ambient = D3DXCOLOR(0.1f, 0.1f, 0.1f, 1.0f);//���̓�����Ȃ������̔��ˌW��
		light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);//���̓����镔���̔��ˌW��

		D3DXVECTOR3 eye = D3DXVECTOR3(0.0f, 5.0f, -5.0f);
		D3DXVECTOR3 at = D3DXVECTOR3(0.0f, 0.0f, 5.0f);
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

		Scene* scene = GameManager::GetScene();

		//�V���h�E�J�����ʒu����
		if (scene->GetGameObject<Player>(DRAW_3D, 0) != NULL)
		{
			D3DXVECTOR3 PlayerPos = scene->GetGameObject<Player>(DRAW_3D, 0)->GetPosition11();
			at = PlayerPos;
			/*PlayerPos.x += 15.0f;*/
			PlayerPos.y += 3.0f;
			PlayerPos.z -= 3.0f;
			eye = PlayerPos;
		}

		//���C�g�J�����̃r���[�s����쐬
		D3DXMatrixLookAtLH(&light.ViewMatrix, &eye, &at, &up);
		//���C�g�J�����̃v���W�F�N�V�����s����쐬
		D3DXMatrixPerspectiveFovLH(&light.ProjectionMatrix, 1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 30.0f);
		//(&light.ProjectionMatrix, 1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 5.0f, 30.0f);
		//���C�g�����Z�b�g
		GetDX11Renderer->SetLight(light);

		m_mainDx11->BeginDepth();

		m_mainDx11->SetViewMatrix(&light.ViewMatrix);
		m_mainDx11->SetProjectionMatrix(&light.ProjectionMatrix);

		GameManager::DrawShadow(version);

		ID3D11RenderTargetView* pView[2];

		pView[0] = NULL;
		pView[1] = NULL;

		GetDX11Renderer->GetDeviceContext()->OMSetRenderTargets(2, pView, NULL);

		ID3D11ShaderResourceView* const pSRV[1] = { NULL };
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(0, 1, pSRV);

		m_mainDx11->Begin();

		//ImGui����
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		GameManager::Draw(0);
		ImGui11::Draw();

		//light.Enable = false;
		//GetDX11Renderer->SetLight(light);

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		ID3D11ShaderResourceView* const pSRV2[2] = { NULL, NULL };
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(0, 2, pSRV2);

		ID3D11RenderTargetView* pView2[1] = { NULL };

		GetDX11Renderer->GetDeviceContext()->OMSetRenderTargets(1, pView2, NULL);
		
		m_mainDx11->End();
	}
	else if (version == IDNO)
	{
		m_mainDx12->Begin();

		//ImGui����
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		GameManager::Draw(1);
		ImGui12::Draw();
		
		ImGui::Render();

		GetDX12Renderer->GetCommandList()->SetDescriptorHeaps(1, GetDX12Renderer->GetImGuiHeap().GetAddressOf());
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), GetDX12Renderer->GetCommandList().Get());

		m_mainDx12->End();
	}
	else
	{
		MessageBoxA(0, "Manager::Draw Error", "Error", MB_ICONERROR);
	}
}
