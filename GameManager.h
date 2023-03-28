#pragma once

#include "Scene.h"

class GameManager
{
private:
	static class Scene* m_Scene;

public:
	static void Init(UINT version);
	static void UnInit(UINT version);
	static void Update(UINT version);
	static void Draw(UINT version);
	static void DrawShadow(UINT version);

	static class Scene* GetScene() { return m_Scene; }

	template<typename T>
	static void SetScene(UINT version)
	{
		if (m_Scene)
		{
			m_Scene->UnInit(version);
			delete m_Scene;
		}

		if (version == 0)
			ImGui11::ImGuiSwitch11 = false;
		else
			ImGui12::ImGuiSwitch12 = false;

		m_Scene = new T();
		m_Scene->Init(version);
	}
};