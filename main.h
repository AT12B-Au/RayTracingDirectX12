#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdexcept>

#define SCREEN_WIDTH (1920) //�E�B���h�E��
#define SCREEN_HEIGHT (1080) //�E�B���h�E����
#define WINDOWNAME "���l�̕׋��m�[�g"

class WindowMain
{
public:
	static LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM);

	static HWND m_HWnd;
	static UINT version;
};