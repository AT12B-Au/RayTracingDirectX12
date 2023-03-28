#include "main.h"
#include "GameManager.h"
#include "LoadingScene.h"
#include "GameScene.h"
#include "input.h"
#include "Transition.h"

#include <thread>

//std::mutex LoadingScene::mtxDraw;
//std::condition_variable LoadingScene::cvDraw;
//bool LoadingScene::stopDrawThread = false;
//
//void LoadingScene::LoadingDX12()
//{
//	while (true)
//	{
//		std::unique_lock<std::mutex> lock(mtxDraw);
//		cvDraw.wait(lock, [] { return stopDrawThread; });
//		if (stopDrawThread)
//		{
//			break;
//		}
//
//		GameScene::Load();
//	}
//}

void LoadingScene::Init(UINT version)
{
	AddGameObject<Transition>(DRAW_2D, version);

	if (version == 0)
	{
		std::thread th(GameScene::Load);	//マルチスレッド
		th.detach();
	}
	else if (version == 1)
	{
		//std::thread th(LoadingScene::LoadingDX12);

		//// LoadingDX12 関数を終了させる
		//{
		//	std::unique_lock<std::mutex> lock(mtxDraw);
		//	stopDrawThread = true;
		//}
		//cvDraw.notify_one();

		//th.join();
		//std::thread th(GameScene::Load);	//マルチスレッド
		//th.detach();
		GameScene::Load();
	}
}

void LoadingScene::UnInit(UINT version)
{
	Scene::UnInit(version);
}

void LoadingScene::Update(UINT version)
{
	Scene::Update(version);

	if (GameScene::GetLoadFinish())
		GameManager::SetScene<GameScene>(version);
}