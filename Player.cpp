#include "AnimationModel11.h"
#include "AnimationModel12.h"
//#include <GamePad.h>
#include "GameObject.h"
#include "RendererManager.h"
#include "GameManager.h"
#include "RootSignature.h"
#include "Player.h"
#include "camera.h"
#include "input.h"

AnimationModel11* Player::m_Model11 = NULL;
AnimationModel12* Player::m_Model12 = NULL;
RootSignature* Player::m_RootSignature = NULL;
//RootSignature* Player::m_RootSignatureShadow = NULL;

void Player::Init(UINT version)
{
	if (version == 0)	//DirectX11
	{
		GetDX11Renderer->CreateVertexShader(&m_VertexShader11, &m_VertexLayout11,
			"shader\\PlayerVS.cso");
		GetDX11Renderer->CreatePixelShader(&m_PixelShader11, "shader\\PlayerPS.cso");

		m_Position11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Rotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
		m_Scale11 = D3DXVECTOR3(0.01f, 0.01f, 0.01f);

		m_ObjectRotation11 = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

		speed11 = 0.05f;
		m_BlendRate11 = 0.0f;
		m_CameraSpeed = 1.0f;
		m_AnimationSpeed = 0.03f;

		m_AnimationNormal11 = "Stay";
		m_AnimationAction11 = "Stay";
	}
	else if (version == 1)	//DirectX12
	{
		m_VertexShader12 = GetDX12Renderer->CreateVertexShader(L"shader\\pixelLightingVS.hlsl");
		m_PixelShader12 = GetDX12Renderer->CreatePixelShader(L"shader\\pixelLightingPS.hlsl");

		//パイプラインステート
		m_PipelineState12 = GetDX12Renderer->CreatePipelineState(m_VertexShader12, m_PixelShader12, m_RootSignature->GetRootSignature());
		
		m_AnimationNormal12 = "Stay";
		m_AnimationAction12 = "Stay";

		m_Position12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Rotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_Scale12 = XMFLOAT3(0.01f, 0.01f, 0.01f);

		m_ObjectRotation12 = XMFLOAT3(0.0f, 0.0f, 0.0f);

		speed12 = 0.05f;
		m_BlendRate12 = 0.0f;
		m_CameraSpeed = 1.0f;
		m_AnimationSpeed = 0.03f;

		//m_GamePad = std::make_unique<GamePad>();
	}
}

void Player::UnInit(UINT version)
{
	if (version == 0)	//DirectX11
	{
		m_VertexLayout11->Release();
		m_VertexShader11->Release();
		m_PixelShader11->Release();

		//m_GamePad.release();

		m_Model11->UnLoad();
	}
	else if (version == 1)	//DirectX12
	{
		m_RootSignature->UnInit();
		m_PipelineState12->Release();
		//m_GamePad.release();

		m_Model12->UnLoad();
	}
}

void Player::Update(UINT version)
{
	if (version == 0)	//DirectX11
	{
		D3DXVECTOR3 PlayerForwardX = this->GetForward11(0);
		D3DXVECTOR3 PlayerForwardY = this->GetForward11(1);
		D3DXVECTOR3 PlayerForwardZ = this->GetForward11(2);
		Scene* m_Scene = GameManager::GetScene();
		Camera* m_Camera = m_Scene->GetGameObject<Camera>(DRAW_CAMERA, 0);
		bool AnimationReturn = true;
		bool KeyPressed = false;

		//GamePad::State state = m_GamePad->GetState(0);

		//コントローラー処理
		//{
		//	if (state.IsConnected())
		//	{
		//		//プレイヤー移動
		//		{
		//			if (state.IsLeftThumbStickUp())		//前移動
		//			{
		//				m_AnimationAction11 = "Run";
		//				float StickRangeY = state.thumbSticks.leftY;

		//				m_Velocity11.x += PlayerForwardZ.x * StickRangeY * speed11;
		//				m_Velocity11.z += PlayerForwardZ.z * StickRangeY * speed11;

		//				m_BlendRate11 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}

		//			if (state.IsLeftThumbStickLeft())	//左移動
		//			{
		//				m_AnimationAction11 = "Run";
		//				float StickRangeX = -state.thumbSticks.leftX;

		//				m_Velocity11.x -= PlayerForwardX.x * StickRangeX * speed11;
		//				m_Velocity11.z -= PlayerForwardX.z * StickRangeX * speed11;

		//				m_BlendRate11 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}
		//			if (state.IsLeftThumbStickDown())	//後ろ移動
		//			{
		//				m_AnimationAction11 = "Run";
		//				float StickRangeY = -state.thumbSticks.leftY;

		//				m_Velocity11.x -= PlayerForwardZ.x * StickRangeY * speed11;
		//				m_Velocity11.z -= PlayerForwardZ.z * StickRangeY * speed11;

		//				m_BlendRate11 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}
		//			if (state.IsLeftThumbStickRight())	//右移動
		//			{
		//				m_AnimationAction11 = "Run";
		//				float StickRangeX = state.thumbSticks.leftX;

		//				m_Velocity11.x += PlayerForwardX.x * StickRangeX * speed11;
		//				m_Velocity11.z += PlayerForwardX.z * StickRangeX * speed11;

		//				m_BlendRate11 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}
		//		}

		//		//カメラ移動
		//		{
		//			if (state.IsRightThumbStickLeft())
		//			{
		//				float StickRangeX = -state.thumbSticks.rightX;
		//				m_Rotation11.y -= 0.1f * StickRangeX * m_CameraSpeed;
		//			}
		//			if (state.IsRightThumbStickRight())
		//			{
		//				float StickRangeX = state.thumbSticks.rightX;

		//				m_Rotation11.y += 0.1f * StickRangeX * m_CameraSpeed;
		//			}
		//		}
		//	}
		//}

		//キーボード処理
		{
			//プレイヤー移動
			{
				if (Input::GetKeyPress('W'))	//前移動
				{
					m_AnimationAction11 = "Run";

					m_Velocity11.x += PlayerForwardZ.x * speed11;
					m_Velocity11.y += PlayerForwardZ.y * speed11;
					m_Velocity11.z += PlayerForwardZ.z * speed11;

					m_BlendRate11 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
				if (Input::GetKeyPress('A'))	//左移動
				{
					m_AnimationAction11 = "Run";

					m_Velocity11.x -= PlayerForwardX.x * speed11;
					m_Velocity11.y -= PlayerForwardX.y * speed11;
					m_Velocity11.z -= PlayerForwardX.z * speed11;

					m_BlendRate11 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
				if (Input::GetKeyPress('S'))	//後ろ移動
				{
					m_AnimationAction11 = "Run";

					m_Velocity11.x -= PlayerForwardZ.x * speed11;
					m_Velocity11.y -= PlayerForwardZ.y * speed11;
					m_Velocity11.z -= PlayerForwardZ.z * speed11;

					m_BlendRate11 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
				if (Input::GetKeyPress('D'))	//右移動
				{
					m_AnimationAction11 = "Run";

					m_Velocity11.x += PlayerForwardX.x * speed11;
					m_Velocity11.y += PlayerForwardX.y * speed11;
					m_Velocity11.z += PlayerForwardX.z * speed11;

					m_BlendRate11 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
			}

			//カメラ移動
			{
				if (Input::GetKeyPress('Q'))
				{
					m_Rotation11.y -= 0.1f * m_CameraSpeed;
				}
				if (Input::GetKeyPress('E'))
				{
					m_Rotation11.y += 0.1f * m_CameraSpeed;
				}
			}
		}

		//重力
		if (m_Position11.y > 0.0f)
			m_Velocity11.y -= 0.01f;

		//アニメション戻る時
		if (AnimationReturn)
			m_BlendRate11 -= m_AnimationSpeed;

		//アニメーション限界
		if (m_BlendRate11 > 1.0f)
		{
			m_BlendRate11 = 1.0f;
		}
		else if (m_BlendRate11 < 0.0f)
		{
			m_BlendRate11 = 0.0f;
			m_AnimationAction11 = "Stay";
		}


		//抵抗
		m_Velocity11.x -= m_Velocity11.x * 0.1f;
		m_Velocity11.y -= m_Velocity11.y * 0.01f;
		m_Velocity11.z -= m_Velocity11.z * 0.1f;

		if (m_Velocity11.x > 1.0f)
			m_Velocity11.x = 1.0f;

		if (m_Velocity11.y > 1.0f)
			m_Velocity11.y = 1.0f;

		if (m_Velocity11.z > 1.0f)
			m_Velocity11.z = 1.0f;

		//移動回転
		{
			D3DXVECTOR3 OldPosition = m_Position11;

			//移動
			m_Position11 += m_Velocity11;

			//ベクトル
			D3DXVECTOR3 vector;
			vector = m_Position11 - OldPosition;

			if (KeyPressed)
			{
				static float OldRotationY = 0.0f;

				//正規化
				D3DXVec3Normalize(&PlayerForwardZ, &PlayerForwardZ);

				D3DXVec3Normalize(&vector, &vector);

				//内積
				float DotValue = 0.0f;
				DotValue = D3DXVec3Dot(&PlayerForwardZ, &vector);
				DotValue = acosf(DotValue);		//答え

				//外積	//時計回りか反時計回りか
				D3DXVECTOR3 CrossValue;
				D3DXVec3Cross(&CrossValue, &PlayerForwardZ,&vector);

				if (CrossValue.y >= 0.0f)
				{
					m_ObjectRotation11.y = DotValue + m_Rotation11.y;
				}
				else
				{
					m_ObjectRotation11.y = -DotValue + m_Rotation11.y;
				}

				if (!isnan(m_ObjectRotation11.y))
				{
					OldRotationY = m_ObjectRotation11.y;
				}
				else
				{
					m_ObjectRotation11.y = OldRotationY;

				}
			}
		}

		m_Frame11++;
	}
	else if (version == 1)	//DirectX12
	{
		XMFLOAT3 PlayerForwardX = this->GetForward12(0);
		XMFLOAT3 PlayerForwardY = this->GetForward12(1);
		XMFLOAT3 PlayerForwardZ = this->GetForward12(2);
		Scene* m_Scene = GameManager::GetScene();
		Camera* m_Camera = m_Scene->GetGameObject<Camera>(DRAW_CAMERA, 1);
		bool AnimationReturn = true;
		bool KeyPressed = false;

		//GamePad::State state = m_GamePad->GetState(0);
		//
		////コントローラー処理
		//{
		//	if (state.IsConnected())
		//	{
		//		//プレイヤー移動
		//		{
		//			if (state.IsLeftThumbStickUp())		//前移動
		//			{
		//				m_AnimationAction12 = "Run";
		//				float StickRangeY = state.thumbSticks.leftY;

		//				m_Velocity12.x += PlayerForwardZ.x * StickRangeY *  speed12;
		//				m_Velocity12.z += PlayerForwardZ.z * StickRangeY *  speed12;

		//				m_BlendRate12 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}

		//			if (state.IsLeftThumbStickLeft())	//左移動
		//			{
		//				m_AnimationAction12 = "Run";
		//				float StickRangeX = -state.thumbSticks.leftX;

		//				m_Velocity12.x -= PlayerForwardX.x * StickRangeX *  speed12;
		//				m_Velocity12.z -= PlayerForwardX.z * StickRangeX *  speed12;

		//				m_BlendRate12 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}
		//			if (state.IsLeftThumbStickDown())	//後ろ移動
		//			{
		//				m_AnimationAction12 = "Run";
		//				float StickRangeY = -state.thumbSticks.leftY;

		//				m_Velocity12.x -= PlayerForwardZ.x * StickRangeY *  speed12;
		//				m_Velocity12.z -= PlayerForwardZ.z * StickRangeY * speed12;

		//				m_BlendRate12 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}
		//			if (state.IsLeftThumbStickRight())	//右移動
		//			{
		//				m_AnimationAction12 = "Run";
		//				float StickRangeX = state.thumbSticks.leftX;

		//				m_Velocity12.x += PlayerForwardX.x * StickRangeX *  speed12;
		//				m_Velocity12.z += PlayerForwardX.z * StickRangeX *  speed12;

		//				m_BlendRate12 += m_AnimationSpeed;
		//				AnimationReturn = false;
		//				KeyPressed = true;
		//			}
		//		}
		//		
		//		//カメラ移動
		//		{
		//			if (state.IsRightThumbStickLeft())
		//			{
		//				float StickRangeX = -state.thumbSticks.rightX;
		//				m_Rotation12.y -= 0.1f * StickRangeX * m_CameraSpeed;
		//			}
		//			if (state.IsRightThumbStickRight())
		//			{
		//				float StickRangeX = state.thumbSticks.rightX;

		//				m_Rotation12.y += 0.1f * StickRangeX * m_CameraSpeed;
		//			}
		//		}
		//	}
		//}
		
		//キーボード処理
		{
			//プレイヤー移動
			{
				if (Input::GetKeyPress('W'))	//前移動
				{
					m_AnimationAction12 = "Run";

					m_Velocity12.x += PlayerForwardZ.x * speed12;
					m_Velocity12.y += PlayerForwardZ.y * speed12;
					m_Velocity12.z += PlayerForwardZ.z * speed12;

					m_BlendRate12 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
				if (Input::GetKeyPress('A'))	//左移動
				{
					m_AnimationAction12 = "Run";

					m_Velocity12.x -= PlayerForwardX.x * speed12;
					m_Velocity12.y -= PlayerForwardX.y * speed12;
					m_Velocity12.z -= PlayerForwardX.z * speed12;

					m_BlendRate12 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
				if (Input::GetKeyPress('S'))	//後ろ移動
				{
					m_AnimationAction12 = "Run";

					m_Velocity12.x -= PlayerForwardZ.x * speed12;
					m_Velocity12.y -= PlayerForwardZ.y * speed12;
					m_Velocity12.z -= PlayerForwardZ.z * speed12;

					m_BlendRate12 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
				if (Input::GetKeyPress('D'))	//右移動
				{
					m_AnimationAction12 = "Run";

					m_Velocity12.x += PlayerForwardX.x * speed12;
					m_Velocity12.y += PlayerForwardX.y * speed12;
					m_Velocity12.z += PlayerForwardX.z * speed12;

					m_BlendRate12 += m_AnimationSpeed;
					AnimationReturn = false;
					KeyPressed = true;
				}
			}
			
			//カメラ移動
			{
				if (Input::GetKeyPress('Q'))
				{
					m_Rotation12.y -= 0.1f * m_CameraSpeed;
				}
				if (Input::GetKeyPress('E'))
				{
					m_Rotation12.y += 0.1f * m_CameraSpeed;
				}
			}
		}

		//重力
		if (m_Position12.y > 0.0f)
			m_Velocity12.y -= 0.01f;

		//アニメション戻る時
		if (AnimationReturn)
			m_BlendRate12 -= m_AnimationSpeed;

		//アニメーション限界
		if (m_BlendRate12 > 1.0f)
		{
			m_BlendRate12 = 1.0f;
		}
		else if (m_BlendRate12 < 0.0f)
		{
			m_BlendRate12 = 0.0f;
			m_AnimationAction12 = "Stay";
		}
			

		//抵抗
		m_Velocity12.x -= m_Velocity12.x * 0.1f;
		m_Velocity12.y -= m_Velocity12.y * 0.01f;
		m_Velocity12.z -= m_Velocity12.z * 0.1f;

		if (m_Velocity12.x > 1.0f)
			m_Velocity12.x = 1.0f;

		if (m_Velocity12.y > 1.0f)
			m_Velocity12.y = 1.0f;

		if (m_Velocity12.z > 1.0f)
			m_Velocity12.z = 1.0f;

		//移動回転
		{
			XMFLOAT3 OldPosition = m_Position12;

			//移動
			m_Position12.x += m_Velocity12.x;
			m_Position12.y += m_Velocity12.y;
			m_Position12.z += m_Velocity12.z;

			//ベクトル
			XMFLOAT3 vector;
			vector.x = m_Position12.x - OldPosition.x;
			vector.y = m_Position12.y - OldPosition.y;
			vector.z = m_Position12.z - OldPosition.z;

			if (KeyPressed)
			{
				static float OldRotationY = 0.0f;

				//正規化
				XMVECTOR normalize = XMVector3Normalize(XMLoadFloat3(&PlayerForwardZ));
				XMStoreFloat3(&PlayerForwardZ, normalize);

				normalize = XMVector3Normalize(XMLoadFloat3(&vector));
				XMStoreFloat3(&vector, normalize);

				//内積
				XMVECTOR dot = XMVector3Dot(XMLoadFloat3(&PlayerForwardZ), XMLoadFloat3(&vector));
				float DotValue;
				XMStoreFloat(&DotValue, dot);
				DotValue = acosf(DotValue);		//答え

				//外積	//時計回りか反時計回りか
				XMVECTOR cross = XMVector3Cross(XMLoadFloat3(&PlayerForwardZ), XMLoadFloat3(&vector));
				XMFLOAT3 CrossValue;
				XMStoreFloat3(&CrossValue, cross);

				if (CrossValue.y >= 0.0f)
				{
					m_ObjectRotation12.y = DotValue  + m_Rotation12.y;
				}	
				else
				{
					m_ObjectRotation12.y = -DotValue  + m_Rotation12.y;
				}

				if (!isnan(m_ObjectRotation12.y))
				{
					OldRotationY = m_ObjectRotation12.y;
				}
				else
				{
					m_ObjectRotation12.y = OldRotationY;

				}
			}
		}
		
		m_Frame12++;
	}
}

void Player::Draw(UINT version)
{
	if (version == 0)	//DirectX11
	{
		auto m_DeviceContext = GetDX11Renderer->GetDeviceContext().Get();

		m_Model11->Update(m_AnimationNormal11.c_str(), m_AnimationAction11.c_str(), m_BlendRate11, m_Frame11);

		//入力レイアウト設定
		m_DeviceContext->IASetInputLayout(m_VertexLayout11.Get());

		//シェーダー設定
		m_DeviceContext->VSSetShader(m_VertexShader11.Get(), NULL, 0);
		m_DeviceContext->PSSetShader(m_PixelShader11.Get(), NULL, 0);

		//ワールドマトリクス設定
		D3DXMATRIX world, scale, rot, trans;
		D3DXMatrixScaling(&scale, m_Scale11.x, m_Scale11.y, m_Scale11.z);
		D3DXMatrixRotationYawPitchRoll(&rot, m_ObjectRotation11.y, m_ObjectRotation11.x , m_ObjectRotation11.z);

		D3DXMatrixTranslation(&trans, m_Position11.x, m_Position11.y, m_Position11.z);
		world = scale * rot * trans;
		GetDX11Renderer->SetWorldMatrix(&world);

		ComPtr<ID3D11ShaderResourceView> ShadowDepthTexture = GetDX11Renderer->GetShadowDepthTexture();
		GetDX11Renderer->GetDeviceContext()->PSSetShaderResources(1, 1, ShadowDepthTexture.GetAddressOf());

		m_Model11->Draw();
	}
	else if (version == 1)	//DirectX12
	{
		auto m_CommandList = GetDX12Renderer->GetCommandList();

		Scene* scene = GameManager::GetScene();
		Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 1);

		m_Model12->Update(m_AnimationNormal12.c_str(), m_AnimationAction12.c_str(), m_BlendRate12,m_Frame12);

		//ルートシグネチャをセット
		m_RootSignature->Draw();

		m_CommandList->SetPipelineState(m_PipelineState12.Get());

		XMMATRIX world, scale, rot, trans;

		scale = XMMatrixScaling(m_Scale12.x, m_Scale12.y, m_Scale12.z);
		rot = XMMatrixRotationRollPitchYaw(m_ObjectRotation12.x, m_ObjectRotation12.y, m_ObjectRotation12.z);
		trans = XMMatrixTranslation(m_Position12.x, m_Position12.y, m_Position12.z);

		world = scale * rot * trans;

		//コンスタントバッファの内容を更新
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

			//ライト更新
			LIGHT12 light;
			light.Enable = true;
			XMFLOAT4 normalize = XMFLOAT4(1.0f, -1.0f, 1.0f, 0.0f);
			light.Direction = XMVector4Normalize(XMLoadFloat4(&normalize));
			light.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			light.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

			GetDX12Renderer->SetLight(&light, m_Model12->GetLightBuffer());
		}

		m_Model12->Draw();
	}
}

void Player::Load(UINT version)
{
	if (version == 0)
	{
		m_Model11 = new AnimationModel11();
		m_Model11->Load("Asset\\model\\PlayerIdle.fbx");
		m_Model11->LoadAnimation("Asset\\model\\PlayerIdle.fbx", "Stay");
		m_Model11->LoadAnimation("Asset\\model\\PlayerRun.fbx", "Run");

		//m_GamePad = std::make_unique<GamePad>();
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

		////影用
		//{
		//	//constantBuffer数
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
		//	m_RootSignatureShadow->InitCustomize(_countof(RootParameters), RootParameters,NULL);
		//}
	}
}