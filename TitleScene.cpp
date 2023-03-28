#include "main.h"
#include "GameManager.h"
#include "TitleScene.h"
#include "LoadingScene.h"
#include "input.h"
#include "TitleLogo.h"
#include "camera.h"
#include "Lake.h"
#include "Moon.h"
#include "TitleText.h"
#include "TitleText1.h"

void TitleScene::Init(UINT version)
{
	ChangeScene = false;

	AddGameObject<Camera>(DRAW_CAMERA, version);
	AddGameObject<Lake>(DRAW_3D, version);
	AddGameObject<Moon>(DRAW_3D, version);
	AddGameObject<TitleText>(DRAW_3D, version);
	AddGameObject<TitleText1>(DRAW_3D, version);
	//AddGameObject<TitleLogo>(DRAW_2D, version);
}

void TitleScene::UnInit(UINT version)
{
	Scene::UnInit(version);
}

void TitleScene::Update(UINT version)
{

	Scene::Update(version);

	static float alpha = 1.0f;

	if (Input::GetKeyTrigger(VK_RETURN))
	{
		ChangeScene = true;
	}

	if (ChangeScene)
	{
		Scene::SetAlpha(version, alpha);
		alpha -= 0.01f;

		if (alpha < 0.0f)
			GameManager::SetScene<LoadingScene>(version);
	}
}