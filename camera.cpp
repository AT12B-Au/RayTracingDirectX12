#include "main.h"
#include "AppDx11.h"
#include "AppDx12.h"
#include "GameObject.h"
#include "GameObjectDx12.h"
#include "input.h"
#include "RendererManager.h"
#include "GameManager.h"
#include "camera.h"
#include "Player.h"
#include "TitleText.h"

void Camera::Init(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_Position11 = D3DXVECTOR3(0.0f, 5.0f, -5.0f);
		m_Target11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		speed11 = 5.0f;
	}
	else if (version == 1)	//DirectX12
	{
		m_Position12 = XMFLOAT3(0.0f, 5.0f, -5.0f);
		m_Target12 = XMFLOAT3(0.0f, 0.0f, 0.0f);

		speed12 = 5.0f;
	}
}

void Camera::UnInit(UINT version)
{
	if (version == 0)	//DirectX11
	{

	}
	else if (version == 1)	//DirectX12
	{

	}
}

void Camera::Update(UINT version)
{
	if (version == 0)	//DirectX11
	{
		Scene* scene = GameManager::GetScene();
		Player* player = scene->GetGameObject<Player>(DRAW_3D,version);
		

		if (player != NULL)
		{
			D3DXVECTOR3 playerPosition = player->GetPosition11();
			D3DXVECTOR3 CameraForwardZ = player->GetForward11(2);

			//サードビュー
			m_Target11 = playerPosition;

			//カメラ調整
			m_Target11.y += 2.0f;

			m_Position11 = playerPosition - CameraForwardZ * speed11 + D3DXVECTOR3(0.0f, 2.0f, 0.0f);
		}
		else
		{
			D3DXVECTOR3 playerPosition = GameManager::GetScene()->GetGameObject<TitleText>(DRAW_3D, version)->GetPosition11();
			D3DXVECTOR3 CameraForwardZ = GameManager::GetScene()->GetGameObject<TitleText>(DRAW_3D, version)->GetForward11(2);

			//サードビュー
			m_Target11 = playerPosition;

			//カメラ調整
			/*m_Target11.x -= 1.0f;*/

			m_Position11 = playerPosition - CameraForwardZ * speed11 /*+ D3DXVECTOR3(-2.0f, 0.0f, 0.0f)*/;
		}
		

		//ファーストビュー
		//m_Target = playerPosition + CameraForwardZ;
		//m_Position = playerPosition;
	}
	else if (version == 1)	//DirectX12
	{
		Scene* scene = GameManager::GetScene();
		Player* player = scene->GetGameObject<Player>(DRAW_3D, version);
		if (player != NULL)
		{
			XMFLOAT3 playerPosition = player->GetPosition12();
			XMFLOAT3 CameraForwardZ = player->GetForward12(2);

			//サードビュー
			m_Target12 = playerPosition;

			//カメラ調整
			m_Target12.y += 2.0f;
			//ダメ
			//m_Position12 = playerPosition - CameraForwardZ * speed12 + XMFLOAT3(0.0f, 2.0f, 0.0f);
			m_Position12.x = playerPosition.x - CameraForwardZ.x * speed12 + 0.0f;
			m_Position12.y = playerPosition.y - CameraForwardZ.y * speed12 + 2.0f;
			m_Position12.z = playerPosition.z - CameraForwardZ.z * speed12 + 0.0f;
		}
		else
		{
			XMFLOAT3 playerPosition = GameManager::GetScene()->GetGameObject<TitleText>(DRAW_3D, version)->GetPosition12();
			XMFLOAT3 CameraForwardZ = GameManager::GetScene()->GetGameObject<TitleText>(DRAW_3D, version)->GetForward12(2);

			//サードビュー
			m_Target12 = playerPosition;

			//ダメ
			//m_Position12 = playerPosition - CameraForwardZ * speed12 + XMFLOAT3(0.0f, 2.0f, 0.0f);
			m_Position12.x = playerPosition.x - CameraForwardZ.x * speed12;
			m_Position12.y = playerPosition.y - CameraForwardZ.y * speed12;
			m_Position12.z = playerPosition.z - CameraForwardZ.z * speed12;
		}

		

		//ファーストビュー
		//m_Target12 = playerPosition + CameraForwardZ;
		/*m_Target12.x = playerPosition.x + CameraForwardZ.x;
		m_Target12.y = playerPosition.y + CameraForwardZ.y;
		m_Target12.z = playerPosition.z + CameraForwardZ.z;
		m_Position12 = playerPosition;*/
	}
}

void Camera::Draw(UINT version)
{
	if (version == 0)	//DirectX11
	{
		//ビューマトリクス設定
		//D3DXMATRIX viewMatrix;
		D3DXVECTOR3 up = D3DXVECTOR3(0.0f, 1.0f, 0.0f);


		D3DXMatrixLookAtLH(&m_ViewMatrix11, &m_Position11, &m_Target11, &up);

		GetDX11Renderer->SetViewMatrix(&m_ViewMatrix11);

		//プロジェクションマトリクス設定
		D3DXMATRIX projectionMatrix;
		D3DXMatrixPerspectiveFovLH(&projectionMatrix, 1.0f,
			(float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 1000.0f);

		GetDX11Renderer->SetProjectionMatrix(&projectionMatrix);

		GlobalBuffer11 gb;
		ZeroMemory(&gb, sizeof(gb));
		gb.g_CameraPosition = D3DXVECTOR4(m_Position11.x, m_Position11.y, m_Position11.z, 1.0f);
		GetDX11Renderer->SetGlobal(gb);
	}
	else if (version == 1)	//DirectX12
	{
		XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

		m_ViewMatrix12 = XMMatrixLookAtLH(XMLoadFloat3(&m_Position12),
			XMLoadFloat3(&m_Target12), XMLoadFloat3(&up));

		//プロジェクションマトリクス設定
		m_ProjMatrix12 = XMMatrixPerspectiveFovLH(1.0f, (float)SCREEN_WIDTH / SCREEN_HEIGHT, 1.0f, 1000.0f);
	}
}