#include "ImGui12.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "RayTracingScene.h"
#include "main.h"
#include "GameManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "input.h"
#include "Player.h"
#include "SkyDome.h"
#include "Plane.h"
#include "Polygon2D.h"
#include "TextPolygon2D.h"
#include "TitleLogo.h"
#include "Moon.h"
#include "Lake.h"
#include "camera.h"

bool ImGui12::ImGuiSwitch12 = false;
bool ImGui12::ChangeTitleScene = false;
bool ImGui12::ChangeGameScene = false;
bool ImGui12::ChangeRayTracingScene = false;
ImVec4 ImGui12::ClearColor = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

template<typename T>	//テンプレート関数
void AddObjectButton12(int i,const char* name)
{
	Scene* scene = GameManager::GetScene();
	static bool button = true;
	auto m_CommandList = GetDX12Renderer->GetCommandList();
	auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
	auto m_SwapChain = GetDX12Renderer->GetSwapChain();

	if (ImGui::Button(name))
	{
		if (button)
		{
			scene->GetGameObject<T>(i, 1)->SetDestroy(1);
			button = false;
		}
		else if (!button)
		{
			GetDX12Renderer->GetCommandList()->Close();

			T::Load(1);

			scene->AddGameObject<T>(i, 1);

			//コマンドリストに書き込む前にはコマンドアロケーターをリセットする
			m_CommandAllocator->Reset();
			//コマンドリストをリセットする
			m_CommandList->Reset(m_CommandAllocator.Get(), nullptr);

			//バックバッファが現在何枚目かを取得
			UINT BackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
			auto m_RTargetBack = GetDX12Renderer->GetRTarget(BackBufferIndex);
			auto tran = CD3DX12_RESOURCE_BARRIER::Transition(m_RTargetBack.Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
			//バックバッファのトランジションをレンダーターゲットモードにする
			m_CommandList->ResourceBarrier(1, &tran);

			button = true;
		}
	}
	ImGui::SameLine();
	if (button)
		ImGui::Text("true");
	else
		ImGui::Text("false");
}

void ImGui12::SetImGuiOnOff12()
{
	Scene* scene = GameManager::GetScene();

	//UIのON / OFF
	if (Input::GetKeyTrigger('K'))
	{
		if (!ImGuiSwitch12)
		{
			ImGuiSwitch12 = true;
			//scene->AddGameObject<TitleLogo>(DRAW_2D, 1);
		}
		else if (ImGuiSwitch12)
		{
			ImGuiSwitch12 = false;
			//scene->GetGameObject<TitleLogo>(DRAW_2D, 1)->SetDestroy(0);
		}

	}
}

void ImGui12::Draw()
{
	if (ImGuiSwitch12)
	{
		Scene* scene = GameManager::GetScene();
		auto m_CommandList = GetDX12Renderer->GetCommandList();
		auto m_CommandAllocator = GetDX12Renderer->GetCommandAllocator();
		auto m_SwapChain = GetDX12Renderer->GetSwapChain();
		
		//シーン管理
		{

			ImGui::Begin("Scene Manage");
			ImGui::SetWindowSize(ImVec2(400, 400), true);

			if (typeid(*scene) == typeid(GameScene))
			{

				AddObjectButton12<SkyDome>(DRAW_3D, "SkyDome");
				AddObjectButton12<Plane>(DRAW_3D, "Plane");

			}

			if (typeid(*scene) != typeid(TitleScene))
			{
				if (ImGui::Button("TitleScene"))
				{
					ImGuiSwitch12 = false;

					ChangeTitleScene = true;
				}
			}
			if (typeid(*scene) != typeid(GameScene))
			{
				if (ImGui::Button("GameScene"))
				{
					ImGuiSwitch12 = false;

					ChangeGameScene = true;
				}
			}

			if (typeid(*scene) != typeid(RayTracingScene))
			{
				if (ImGui::Button("RayTracingScene"))
				{
					ImGuiSwitch12 = false;

					ChangeRayTracingScene = true;
				}
			}

			ImGui::End();
		}

		//システム管理
		{
			ImGui::Begin("System Manage");
			ImGui::SetWindowSize(ImVec2(400, 400), true);

			//FPS
			ImGuiIO& io = ImGui::GetIO();
			float fps = io.Framerate;
			ImGui::Text("FPS: %.1f", fps);

			// FPS履歴を格納するための配列
			static float FpsHistory[100];
			static int FpsOffset = 0;

			// FPSを配列に格納
			FpsHistory[FpsOffset] = fps;
			FpsOffset = (FpsOffset + 1) % 100;
			ImGui::PlotLines("FPS", FpsHistory, 100, FpsOffset, NULL, 0, 100, ImVec2(0, 80));
    		
			ImGui::End();
		}

		{
			ImGui::Begin("Normal");
			ImGui::SetWindowSize(ImVec2(400, 200), true);

			ImGui::ColorEdit3("clear color", (float*)&ClearColor);

			ImGui::End();
		}

		Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 1);

		if (typeid(*scene) != typeid(RayTracingScene))
		{
			//Camera
			if (camera != NULL)
			{


				ImGui::Begin("Camera");
				ImGui::SetWindowSize(ImVec2(400, 200), true);
				ImGui::Text("Position:%.1f,%.1f,%.1f",
					camera->GetPosition12().x, camera->GetPosition12().y, camera->GetPosition12().z);
				ImGui::Text("Rotation:%.1f,%.1f,%.1f",
					camera->GetRotation12().x, camera->GetRotation12().y, camera->GetRotation12().z);

				ImGui::End();
			}
		}

		//SceneObject
		{
			if (typeid(*scene) != typeid(RayTracingScene))
			{
				Moon* moon = scene->GetGameObject<Moon>(DRAW_3D, 1);
				Lake* lake = scene->GetGameObject<Lake>(DRAW_3D, 1);
				Player* player = scene->GetGameObject<Player>(DRAW_3D, 1);

				if (scene->GetGameObject<Moon>(DRAW_3D, 1) != NULL)
				{
					ImGui::Begin("Moon");
					ImGui::SetWindowSize(ImVec2(400, 200), true);
					ImGui::Text("Position:%.1f,%.1f,%.1f",
						moon->GetPosition12().x, moon->GetPosition12().y, moon->GetPosition12().z);
					ImGui::Text("Rotation:%.1f,%.1f,%.1f",
						moon->GetRotation12().x, moon->GetRotation12().y, moon->GetRotation12().z);

					ImGui::End();
				}

				if (scene->GetGameObject<Lake>(DRAW_3D, 1) != NULL)
				{
					ImGui::Begin("Lake");
					ImGui::SetWindowSize(ImVec2(400, 200), true);
					ImGui::Text("Position:%.1f,%.1f,%.1f",
						lake->GetPosition12().x, lake->GetPosition12().y, lake->GetPosition12().z);
					ImGui::Text("Rotation:%.1f,%.1f,%.1f",
						lake->GetRotation12().x, lake->GetRotation12().y, lake->GetRotation12().z);

					ImGui::End();
				}

				if (scene->GetGameObject<Player>(DRAW_3D, 1) != NULL)
				{
					ImGui::Begin("Player");
					ImGui::SetWindowSize(ImVec2(400, 200), true);
					ImGui::Text("Position:%.1f,%.1f,%.1f",
						player->GetPosition12().x, player->GetPosition12().y, player->GetPosition12().z);
					ImGui::Text("Rotation:%.1f,%.1f,%.1f",
						player->GetRotation12().x, player->GetRotation12().y, player->GetRotation12().z);

					ImGui::End();
				}
			}
		}
	}
}

void ImGui12::Update()
{
	if (ChangeTitleScene)
	{
		ChangeTitleScene = false;
		GameManager::SetScene<TitleScene>(1);
	}

	if (ChangeGameScene)
	{
		ChangeGameScene = false;
		GameScene::Load();
		GameManager::SetScene<GameScene>(1);
	}

	if (ChangeRayTracingScene)
	{
		ChangeRayTracingScene = false;
		GameManager::SetScene<RayTracingScene>(1);
	}
}