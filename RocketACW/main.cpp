#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "DirectXManager.h"
#include "Game.h"
#include <Keyboard.h>
#include <chrono>
#include "AntTweakManager.h"
#include "Result.h"


HINSTANCE				gHInst = nullptr;
HWND					gHWnd = nullptr;

HRESULT InitWindow(HINSTANCE pHInstance, int pNCmdShow);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(_In_ const HINSTANCE pHInstance, _In_opt_ const HINSTANCE pHPrevInstance, _In_ const LPWSTR pLpCmdLine, _In_ const int pNCmdShow)
{
	UNREFERENCED_PARAMETER(pHPrevInstance);
	UNREFERENCED_PARAMETER(pLpCmdLine);

	if (FAILED(InitWindow(pHInstance, pNCmdShow)))
		return 0;

	RECT rc;
	GetClientRect(gHWnd, &rc);
	const UINT width = rc.right - rc.left;
	const UINT height = rc.bottom - rc.top;

	AntTweakManager antTweakManager;
	DirectXManager dXManager(gHWnd, antTweakManager);
	Game game(width,height, antTweakManager);
	auto lastTime = std::chrono::high_resolution_clock::now();
	// Main message loop
	MSG msg = { nullptr };
	while (WM_QUIT != msg.message)
	{
		//find deltatime
		const auto time = std::chrono::high_resolution_clock::now();
		const auto timestep = time - lastTime;
		const auto dt = (timestep).count() / 1e+9;
	
		auto hr{ Result::OK };
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			//if game has recorded an exit command then cleanup and exit
			if (game.Exit())
			{
				dXManager.Cleanup();
				antTweakManager.Cleanup();
				return static_cast<int>(msg.wParam);
			}

			//update and render
			game.Update(dt);
			hr = dXManager.Render(game.GameObjects(), game.Cam(), game.Lights(), game.ScaledTime());
			lastTime = time;
			if (FAILED(hr))
			{
				dXManager.Cleanup();
				antTweakManager.Cleanup();
				return static_cast<int>(msg.wParam);
			}

		}

	}

	
	dXManager.Cleanup();
	antTweakManager.Cleanup();
	return static_cast<int>(msg.wParam);
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow(const HINSTANCE pHInstance, const int pNCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = pHInstance;
	wcex.hIcon = LoadIcon(pHInstance, nullptr);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"RocketACW";
	wcex.hIconSm = LoadIcon(wcex.hInstance, nullptr);
	if (!RegisterClassEx(&wcex))
		return Result::FAIL;

	// Create window
	gHInst = pHInstance;
	RECT rc = { 0, 0, 800, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	gHWnd = CreateWindow(
		L"RocketACW", 
		L"Rocket Test",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, pHInstance,
		nullptr);
	if (!gHWnd)
		return Result::FAIL;

	ShowWindow(gHWnd, pNCmdShow);

	return Result::OK;
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(const HWND pHWnd, const UINT pMessage, const WPARAM pWParam, const LPARAM pLParam)
{
	//PAINTSTRUCT ps;
	//HDC hdc;

	// Send event message to AntTweakBar
	if (TwEventWin(pHWnd, pMessage, pWParam, pLParam))
		return 0; // Event has been handled by AntTweakBar

	switch (pMessage)
	{
	//case WM_PAINT:
	//	hdc = BeginPaint(pHWnd, &ps);
	//	EndPaint(pHWnd, &ps);
	//	break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_ACTIVATEAPP:
		
		DirectX::Keyboard::ProcessMessage(pMessage, pWParam, pLParam);
		break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		DirectX::Keyboard::ProcessMessage(pMessage, pWParam, pLParam);
		break;
		// Note that this tutorial does not handle resizing (WM_SIZE) requests,
		// so we created the window without the resize border.

	default:
		return DefWindowProc(pHWnd, pMessage, pWParam, pLParam);
	}

	return 0;
}



