// oledSaverWin.cpp : Defines the entry point for the application.
//

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
#define UNICODE
#define _UNICODE

//  Windows Header Files:
#include <windows.h>


#include "resource.h"

// Global Variables:
HINSTANCE hInst; // current instance

static LPCWSTR szWindowClass = L"oledSaverWinClass";

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void OnFullScreen(HWND hwnd, int x, int y, UINT keyFlags);


int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OLEDSAVERWIN));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_APPWINDOW, szWindowClass, L"", WS_POPUP,
						  CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	SetLayeredWindowAttributes(hWnd, 0, 240, LWA_ALPHA);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//   OnFullScreen(hWnd, 0, 0, 0);

	return TRUE;
}

WINDOWPLACEMENT g_wpPrev = {sizeof(g_wpPrev)};

BOOL isFullscreen = FALSE;

void OnFullScreen(HWND hwnd, int x, int y, UINT keyFlags)
{
	if (!isFullscreen)
	{
		MONITORINFO mi = {sizeof(mi)};
		if (GetWindowPlacement(hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
		{
			SetWindowPos(hwnd, HWND_TOP,
						 mi.rcMonitor.left, mi.rcMonitor.top,
						 mi.rcMonitor.right - mi.rcMonitor.left,
						 mi.rcMonitor.bottom - mi.rcMonitor.top,
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		}
		// and hide mouse coursor
		ShowCursor(false);
		isFullscreen = TRUE;
	}
	else
	{
		SetWindowPlacement(hwnd, &g_wpPrev);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		ShowCursor(true);
		isFullscreen = FALSE;
	}
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	static BOOL isDragging = FALSE;
	static BOOL isResizing = FALSE;
	static POINT dragStartPos = {};
	static POINT dragOffset = {};
	static SIZE sizeStart = {};

	switch (message)
	{
		//	case WM_CREATE:
		//	{
		//		HICON hIcon = LoadIcon(NULL, IDI_WINLOGO);
		//		SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		//		SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		//	}
		//	break;
	case WM_LBUTTONDBLCLK:
		OnFullScreen(hWnd, 0, 0, 0);
		break;
	case WM_KEYDOWN: // on escape
		if (wParam == VK_ESCAPE)
		{
			// if not full screen, then minimize
			if (!isFullscreen)
				ShowWindow(hWnd, SW_MINIMIZE);
			else
				OnFullScreen(hWnd, 0, 0, 0);
		}
		else if (wParam == VK_RETURN)
		{
			OnFullScreen(hWnd, 0, 0, 0);
		}
		break;
	case WM_LBUTTONDOWN:
	{
		isDragging = TRUE;
		// if shift is pressed, then resize
		short shiftState = GetKeyState(VK_SHIFT);
		short controlState = GetKeyState(VK_CONTROL);
		if ((shiftState & 0x8000) || (controlState & 0x8000))
			isResizing = TRUE;
		dragStartPos.x = LOWORD(lParam);
		dragStartPos.y = HIWORD(lParam);
		// client to screen
		ClientToScreen(hWnd, &dragStartPos);
		RECT rect;
		GetWindowRect(hWnd, &rect);
		dragOffset.x = rect.left;
		dragOffset.y = rect.top;
		sizeStart.cx = rect.right - rect.left;
		sizeStart.cy = rect.bottom - rect.top;
	}
	break;
	case WM_LBUTTONUP:
		isDragging = FALSE;
		isResizing = FALSE;
		break;
	case WM_MOUSEMOVE:
		if (isDragging && !isFullscreen)
		{
			POINT dragPos = {};
			dragPos.x = LOWORD(lParam);
			dragPos.y = HIWORD(lParam);
			// client to screen
			ClientToScreen(hWnd, &dragPos);

			if (isResizing)
			{
				int newWidth = dragPos.x - dragStartPos.x + sizeStart.cx;
				int newHeight = dragPos.y - dragStartPos.y + sizeStart.cy;
				SetWindowPos(hWnd, HWND_TOP, dragOffset.x, dragOffset.y, newWidth, newHeight, SWP_NOZORDER);
			}
			else
			{
				// move window
				int newX = dragPos.x - dragStartPos.x + dragOffset.x;
				int newY = dragPos.y - dragStartPos.y + dragOffset.y;
				// move
				SetWindowPos(hWnd, HWND_TOP, newX, newY, 0, 0, SWP_NOSIZE);
			}
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
