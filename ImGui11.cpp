#include "Imgui11.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "main.h"
#include "input.h"
#include "GameManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "Player.h"
#include "SkyDome.h"
#include "Plane.h"
#include "TitleLogo.h"
#include "Moon.h"
#include "Lake.h"
#include "camera.h"

bool ImGui11::ImGuiSwitch11 = false;
bool ImGui11::ChangeTitleScene = false;
bool ImGui11::ChangeGameScene = false;
ImVec4 ImGui11::ClearColor = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);

template<typename T>	//テンプレート関数
void AddObjectButton11(int i, const char* name)
{
	Scene* scene = GameManager::GetScene();
	static bool button = true;

	if (ImGui::Button(name))
	{
		if (button)
		{
			scene->GetGameObject<T>(i, 0)->SetDestroy(0);
			button = false;
		}
		else if (!button)
		{
			T::Load(0);
			scene->AddGameObject<T>(i, 0);

			button = true;
		}
	}
	ImGui::SameLine();
	if (button)
		ImGui::Text("true");
	else
		ImGui::Text("false");
}

void ImGui11::SetImGuiOnOff11()
{
	Scene* scene = GameManager::GetScene();

	//UIのON / OFF
	if (Input::GetKeyTrigger('K'))
	{
		if (!ImGuiSwitch11)
		{
			ImGuiSwitch11 = true;
			scene->AddGameObject<TitleLogo>(DRAW_2D, 0);
		}
		else if (ImGuiSwitch11)
		{
			ImGuiSwitch11 = false;
			scene->GetGameObject<TitleLogo>(DRAW_2D, 0)->SetDestroy(0);
		}
			
	}
}

void ImGui11::Draw()
{
	Scene* scene = GameManager::GetScene();

	//常駐UI
	if (ImGuiSwitch11)
	{
		//常駐UI
		{
			ImGui::Begin("Scene Manage");
			ImGui::SetWindowSize(ImVec2(400, 400), true);

			if (typeid(*scene) == typeid(GameScene))
			{
				AddObjectButton11<SkyDome>(DRAW_3D, "SkyDome");
				AddObjectButton11<Plane>(DRAW_3D, "Plane");
			}

			if (typeid(*scene) != typeid(TitleScene))
			{
				if (ImGui::Button("TitleScene"))
				{
					ImGuiSwitch11 = false;

					ChangeTitleScene = true;
				}
			}
			if (typeid(*scene) != typeid(GameScene))
			{
				if (ImGui::Button("GameScene"))
				{
					ImGuiSwitch11 = false;

					ChangeGameScene = true;
				}
			}

			ImGui::End();
		}

		//システム管理
		{
			ImGui::Begin("System Manage");
			ImGui::SetWindowSize(ImVec2(400, 200), true);

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

		Scene* scene = GameManager::GetScene();

		//Camera
		{
			Camera* camera = scene->GetGameObject<Camera>(DRAW_CAMERA, 0);

			ImGui::Begin("Camera");
			ImGui::SetWindowSize(ImVec2(400, 200), true);
			ImGui::Text("Position:%.1f,%.1f,%.1f",
				camera->GetPosition11().x, camera->GetPosition11().y, camera->GetPosition11().z);
			ImGui::Text("Rotation:%.1f,%.1f,%.1f",
				camera->GetRotation11().x, camera->GetRotation11().y, camera->GetRotation11().z);

			ImGui::End();
		}

		//SceneObject
		{
			Moon* moon = scene->GetGameObject<Moon>(DRAW_3D, 0);
			Lake* lake = scene->GetGameObject<Lake>(DRAW_3D, 10);
			Player* player = scene->GetGameObject<Player>(DRAW_3D, 0);

			if (scene->GetGameObject<Moon>(DRAW_3D, 0) != NULL)
			{
				ImGui::Begin("Moon");
				ImGui::SetWindowSize(ImVec2(400, 200), true);
				ImGui::Text("Position:%.1f,%.1f,%.1f",
					moon->GetPosition11().x, moon->GetPosition11().y, moon->GetPosition11().z);
				ImGui::Text("Rotation:%.1f,%.1f,%.1f",
					moon->GetRotation11().x, moon->GetRotation11().y, moon->GetRotation11().z);

				ImGui::End();
			}

			if (scene->GetGameObject<Lake>(DRAW_3D, 0) != NULL)
			{
				ImGui::Begin("Lake");
				ImGui::SetWindowSize(ImVec2(400, 200), true);
				ImGui::Text("Position:%.1f,%.1f,%.1f",
					lake->GetPosition11().x, lake->GetPosition11().y, lake->GetPosition11().z);
				ImGui::Text("Rotation:%.1f,%.1f,%.1f",
					lake->GetRotation11().x, lake->GetRotation11().y, lake->GetRotation11().z);

				ImGui::End();
			}

			if (scene->GetGameObject<Player>(DRAW_3D, 0) != NULL)
			{
				ImGui::Begin("Player");
				ImGui::SetWindowSize(ImVec2(400, 200), true);
				ImGui::Text("Position:%.1f,%.1f,%.1f",
					player->GetPosition11().x, player->GetPosition11().y, player->GetPosition11().z);
				ImGui::Text("Rotation:%.1f,%.1f,%.1f",
					player->GetRotation11().x, player->GetRotation11().y, player->GetRotation11().z);

				ImGui::End();
			}
		}
	}
}

void ImGui11::Update()
{
	if (ChangeTitleScene)
	{
		ChangeTitleScene = false;
		GameManager::SetScene<TitleScene>(0);
	}

	if (ChangeGameScene)
	{
		ChangeGameScene = false;
		GameScene::Load();
		GameManager::SetScene<GameScene>(0);
	}
}