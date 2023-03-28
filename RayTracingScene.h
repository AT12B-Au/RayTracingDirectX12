#pragma once

#include "Scene.h"
#include "RayTracingControl.h"

class RayTracingScene : public Scene
{
private:

	RayTracingControl* m_RT_Control;

public:
	void Init(UINT version);
	void UnInit(UINT version);
	void Update(UINT version);

	void Draw(UINT version);
};