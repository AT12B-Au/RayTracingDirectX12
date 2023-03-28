#include "main.h"
#include "RendererManager.h"
#include "GameManager.h"
#include <stdexcept>
#include <iostream>

HWND WindowMain::m_HWnd = NULL;
UINT WindowMain::version = NULL;


LRESULT CALLBACK WndProc(HWND HWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//
//アプリケーションのエントリー関数
INT WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE, _In_ LPSTR, _In_ INT)
{	
	//ウィンドウの作成
	WNDCLASSEX  wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = (LPSTR)WINDOWNAME;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&wc);

	WindowMain::m_HWnd = CreateWindow((LPSTR)WINDOWNAME, (LPSTR)WINDOWNAME, WS_OVERLAPPEDWINDOW,
		0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, hInstance, 0);
	if (!WindowMain::m_HWnd)
	{
		return E_FAIL;
	}

	ShowWindow(WindowMain::m_HWnd, SW_SHOW);
	UpdateWindow(WindowMain::m_HWnd);

	WindowMain::version = MessageBoxA(0, "DirectX11起動 = YES / はい\nDirectX12起動 = NO / いいえ", "DirectX起動バージョンを選んでください", MB_YESNOCANCEL);

	if (WindowMain::version != IDCANCEL)
	{
		try
		{
			RendererManager::Init(WindowMain::version);
		}
		catch (std::runtime_error const& e)
		{
			AllocConsole();
			freopen("CONOUT$", "w", stdout);
			std::cout << "Error: " << e.what() << "\n";
			SetConsoleCtrlHandler(NULL, true);
			while (true) continue;
		}
	}	
	else return 0;
	
	//Imgui
	{
		if (ImGui::CreateContext() == nullptr)
		{
			assert(0);
			return false;
		}

		if (WindowMain::version == IDYES)
		{
			bool blnResult = ImGui_ImplWin32_Init(WindowMain::m_HWnd);
			if (!blnResult) {
				assert(0);
				return false;
			}
			blnResult = ImGui_ImplDX11_Init(GetDX11Renderer->GetDevice().Get(),
				GetDX11Renderer->GetDeviceContext().Get()
			);
		}
		else if (WindowMain::version == IDNO)
		{
			bool blnResult = ImGui_ImplWin32_Init(WindowMain::m_HWnd);
			if (!blnResult) {
				assert(0);
				return false;
			}
			auto m_Device = GetDX12Renderer->GetDevice();

			blnResult = ImGui_ImplDX12_Init(m_Device.Get(),
				3,
				DXGI_FORMAT_R8G8B8A8_UNORM,
				GetDX12Renderer->GetImGuiHeap().Get(),
				GetDX12Renderer->GetImGuiHeap()->GetCPUDescriptorHandleForHeapStart(),
				GetDX12Renderer->GetImGuiHeap()->GetGPUDescriptorHandleForHeapStart()
			);
		}
	}
	
	
	DWORD dwExecLastTime;
	DWORD dwCurrentTime;
	timeBeginPeriod(1);
	dwExecLastTime = timeGetTime();
	dwCurrentTime = 0;

	MSG msg = { 0 };
	ZeroMemory(&msg, sizeof(msg));

	try
	{
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				if (WindowMain::version == IDYES)
				{

					dwCurrentTime = timeGetTime();

					if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))			//毎フレーム実行回数
					{
						dwExecLastTime = dwCurrentTime;

						RendererManager::Update(WindowMain::version);
						RendererManager::Draw(WindowMain::version);
					}
				}
				else if (WindowMain::version == IDNO)
				{
					dwCurrentTime = timeGetTime();

					if ((dwCurrentTime - dwExecLastTime) >= (1000 / 60))			//毎フレーム実行回数
					{
						dwExecLastTime = dwCurrentTime;
						RendererManager::Update(WindowMain::version);
						RendererManager::Draw(WindowMain::version);
					}
				}
			}
		}

		timeEndPeriod(1);

		UnregisterClass(WINDOWNAME, wc.hInstance);

		RendererManager::UnInit(WindowMain::version);
	}
	catch (std::runtime_error const& e)
	{
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		std::cout << "Error: " << e.what() << "\n";
		while (true) continue;
	}

	return static_cast<int>(msg.wParam);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

//
//ウィンドウプロシージャー
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_DESTROY)
	{
		PostQuitMessage(0);
		return 0;
	}

	ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	return WindowMain::MsgProc(hWnd, uMsg, wParam, lParam);
}
LRESULT WindowMain::MsgProc(HWND hWnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	switch (iMsg)
	{
	case WM_KEYDOWN:
		switch ((char)wParam)
		{
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hWnd, iMsg, wParam, lParam);
}