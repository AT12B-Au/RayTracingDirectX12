#pragma once
#include "imgui/imgui.h"

class ImGui12 {
private:

	static bool ChangeTitleScene;
	static bool ChangeGameScene;
	static bool ChangeRayTracingScene;

public:

	static bool ImGuiSwitch12;
	static ImVec4 ClearColor;

	static void SetImGuiOnOff12();
	static void Draw();
	static void Update();
};