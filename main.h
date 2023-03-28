#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdexcept>

#define SCREEN_WIDTH (1920) //ウィンドウ幅
#define SCREEN_HEIGHT (1080) //ウィンドウ高さ
#define WINDOWNAME "王様の勉強ノート"

class WindowMain
{
public:
	static LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM);

	static HWND m_HWnd;
	static UINT version;
};