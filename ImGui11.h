#pragma once
#include "imgui/imgui.h"

class ImGui11 {

private:
	static bool ChangeTitleScene;
	static bool ChangeGameScene;
public:

	static bool ImGuiSwitch11;
	static ImVec4 ClearColor;

	static void SetImGuiOnOff11();
	static void Draw();
	static void Update();

};