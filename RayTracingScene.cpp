#include "RayTracingScene.h"
#include "Ball.h"
#include "Box.h"
#include "Floor.h"
#include "PointLight.h"
#include "Pillar.h"
#include "Cacti.h"
#include "RayTracingControl.h"

void RayTracingScene::Init(UINT version)
{
	if (version == 1)		//DirectX12�̂�
	{
		//���DescHeap��p�ӂ���
		{
			RayTracingControl::InitDescHeap();
		}

		//�V�[���̃I�u�W�F�N�g�Ǘ�
		{
			AddGameObject<Ball>(DRAW_RAYTRACING, version);
			AddGameObject<Box>(DRAW_RAYTRACING, version);
			AddGameObject<Floor>(DRAW_RAYTRACING, version);
			AddGameObject<Pillar>(DRAW_RAYTRACING, version);
			AddGameObject<Cacti>(DRAW_RAYTRACING, version);
		}

		//���C�g
		{
			AddGameObject<PointLight>(DRAW_RAYTRACING, version);
		}

		//�Ǘ���������
		{
			RayTracingControl::Init();
		}
	}
}

void RayTracingScene::UnInit(UINT version)
{
	if (version == 1)		//DirectX12�̂�
	{
		Scene::UnInit(version);
		
		RayTracingControl::UnInit();
	}
}

void RayTracingScene::Update(UINT version)
{
	if (version == 1)		//DirectX12�̂�
	{
		Scene::Update(version);

		RayTracingControl::Update();
	}
}

void RayTracingScene::Draw(UINT version)
{
	if (version == 0)
		ImGui11::SetImGuiOnOff11();
	else if (version == 1)
		ImGui12::SetImGuiOnOff12();

	RayTracingControl::Begin();

	#pragma omp parallel for
	for (int i = 0; i < TOTAL_DRAW; i++)
	{
		#pragma omp parallel for
		for (GameObject* object : m_GameObject[i])
		{
			//GetDX12Renderer->Begin();
			object->Draw(version);
			//GetDX12Renderer->End();
		}
	}

	RayTracingControl::End();
}
