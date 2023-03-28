#pragma once

#include "Scene.h"

class TestScene : public Scene
{
public:
	void Init(UINT version);
	void UnInit(UINT version);
	void Update(UINT version);
};