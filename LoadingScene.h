#pragma once
#include "Scene.h"
#include <condition_variable>
#include <mutex>

class LoadingScene : public Scene
{
private:
	static std::mutex mtxDraw;
	static std::condition_variable cvDraw;
	static bool stopDrawThread;

public:
	static void LoadingDX12();

	void Init(UINT version);
	void UnInit(UINT version);
	void Update(UINT version);
};