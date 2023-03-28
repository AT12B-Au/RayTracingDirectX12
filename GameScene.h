#pragma once

#include "Scene.h"

class GameScene : public Scene
{
private:

	static bool m_LoadFinish;

public:

	void Init(UINT version) override;
	void UnInit(UINT version) override;
	void Update(UINT version) override;

	static void Load();
	static void LoadAsync();
	static bool GetLoadFinish() { return m_LoadFinish; }
};