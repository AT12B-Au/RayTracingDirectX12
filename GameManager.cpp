#include "AppDx12.h"
#include <Windows.h>
#include "GameManager.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "TestScene.h"
#include "RayTracingScene.h"
#include "input.h"

Scene* GameManager::m_Scene = NULL;

void GameManager::Init(UINT version)
{
	Input::Init();

	//GameScene::Load();

	//SetScene<GameScene>(version);
	//SetScene<TitleScene>(version);
	//SetScene<TestScene>(version);
	if (version == 0)
		SetScene<TitleScene>(version);
	if (version == 1)
		SetScene<RayTracingScene>(version);
}

void GameManager::UnInit(UINT version)
{
	Input::UnInit();

	m_Scene->UnInit(version);
	delete m_Scene;
}

void GameManager::Update(UINT version)
{
	Input::Update();

	m_Scene->Update(version);
}

void GameManager::Draw(UINT version)
{
	if (typeid(*m_Scene) == typeid(RayTracingScene))
	{
		if (version == 1)
			m_Scene->Draw(version);
	}
	else
	{
		m_Scene->Draw(version);
	}
}

void GameManager::DrawShadow(UINT version)
{
	m_Scene->DrawShadow(version);
}
