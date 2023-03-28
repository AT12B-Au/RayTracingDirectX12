#pragma once

#include "Scene.h"

class TitleScene : public Scene
{
private:

	bool ChangeScene;

public:
	void Init(UINT version);
	void UnInit(UINT version);
	void Update(UINT version);
};