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
static LPCWSTR szWindowTitle = L"oledSaverWin";
static const int nAlphaValue = 240;

// Forward declarations of functions included in this code module:
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void FullscreenHandler(HWND hwnd, WPARAM wParam);

int APIENTRY WinMain(HINSTANCE hInstance,
					 HINSTANCE hPrevInstance,
					 LPSTR lpCmdLine,
					 int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

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

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable

	hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_APPWINDOW, szWindowClass, szWindowTitle, WS_POPUP,
						  CW_USEDEFAULT, CW_USEDEFAULT, 600, 400, NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	SetLayeredWindowAttributes(hWnd, 0, nAlphaValue, LWA_ALPHA);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//	FullscreenHandler(hWnd, VK_RETURN);	// emulate enter hit

	return TRUE;
}

enum DragResizeEvent
{
	dreStart = 0,
	dreStop,
	dreMove,
	dreFullscreenOn,
	dreFullscreenOff,
};

void DragHandler(HWND hwnd, DragResizeEvent event)
{
	static bool isFullscreen = false;
	static bool isDragging = false;
	static POINT dragStartPos = {};
	static POINT windowStartPos = {};

	if (dreStop == event)
	{
		isDragging = false;
		return;
	}
	else if (dreFullscreenOn == event)
	{
		isFullscreen = true;
		return;
	}
	else if (dreFullscreenOff == event)
	{
		isFullscreen = false;
		return;
	}
	else if (dreStart == event)
	{
		if (isFullscreen)
			return;

		short shiftState = GetKeyState(VK_SHIFT);
		short controlState = GetKeyState(VK_CONTROL);
		if ((shiftState & 0x8000) || (controlState & 0x8000))
			return; // shift or control -> resizing, not dragging

		isDragging = true;
		GetCursorPos(&dragStartPos);
		RECT rect;
		GetWindowRect(hwnd, &rect);
		windowStartPos.x = rect.left;
		windowStartPos.y = rect.top;
		return;
	}
	else if (dreMove == event)
	{
		if (!isDragging)
			return;

		POINT dragPos = {};
		GetCursorPos(&dragPos);
		// move window
		int newX = dragPos.x - dragStartPos.x + windowStartPos.x;
		int newY = dragPos.y - dragStartPos.y + windowStartPos.y;
		SetWindowPos(hwnd, NULL, newX, newY, 0, 0, SWP_NOSIZE);
		return;
	}
}

void ResizeHandler(HWND hwnd, DragResizeEvent event)
{
	static bool isFullscreen = false;
	static bool isResizing = false;
	static POINT dragStartPos = {};
	static POINT windowStartSize = {};

	if (dreStop == event)
	{
		isResizing = false;
		return;
	}
	else if (dreFullscreenOn == event)
	{
		isFullscreen = true;
		return;
	}
	else if (dreFullscreenOff == event)
	{
		isFullscreen = false;
		return;
	}
	else if (dreStart == event)
	{
		if (isFullscreen)
			return;

		short shiftState = GetKeyState(VK_SHIFT);
		short controlState = GetKeyState(VK_CONTROL);
		if (!(shiftState & 0x8000) && !(controlState & 0x8000))
			return; // no shift or control -> dragging, not resizing

		isResizing = true;
		GetCursorPos(&dragStartPos);
		RECT rect;
		GetWindowRect(hwnd, &rect);
		windowStartSize.x = rect.right - rect.left;
		windowStartSize.y = rect.bottom - rect.top;
		return;
	}
	else if (dreMove == event)
	{
		if (!isResizing)
			return;

		POINT dragPos = {};
		GetCursorPos(&dragPos);
		// resize window
		int newWidth = dragPos.x - dragStartPos.x + windowStartSize.x;
		if (newWidth < 100)
			newWidth = 100;
		int newHeight = dragPos.y - dragStartPos.y + windowStartSize.y;
		if (newHeight < 100)
			newHeight = 100;
		SetWindowPos(hwnd, HWND_TOP, 0, 0, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE);
	}
}

void FullscreenHandler(HWND hwnd, WPARAM wParam)
{
	static bool isFullscreen = false;
	static WINDOWPLACEMENT wpPrev = {sizeof(wpPrev)};

	// handle escape and enter keys only
	if ((VK_RETURN != wParam) && (VK_ESCAPE != wParam))
		return;

	if ((VK_ESCAPE == wParam) && !isFullscreen)
	{
		// if not full screen, then close window
		PostMessage(hwnd, WM_CLOSE, 0, 0);
		return;
	}

	// otherwise, toggle
	if (!isFullscreen)
	{
		MONITORINFO mi = {sizeof(mi)};
		if (!GetWindowPlacement(hwnd, &wpPrev) || !GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi))
			return;

		SetWindowPos(hwnd, HWND_TOP,
					 mi.rcMonitor.left, mi.rcMonitor.top,
					 mi.rcMonitor.right - mi.rcMonitor.left,
					 mi.rcMonitor.bottom - mi.rcMonitor.top,
					 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		// and hide mouse coursor
		ShowCursor(false);
		isFullscreen = true;
		DragHandler(hwnd, dreFullscreenOn);
		ResizeHandler(hwnd, dreFullscreenOn);
	}
	else
	{
		SetWindowPlacement(hwnd, &wpPrev);
		SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
					 SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
						 SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		ShowCursor(true);
		isFullscreen = false;
		DragHandler(hwnd, dreFullscreenOff);
		ResizeHandler(hwnd, dreFullscreenOff);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_LBUTTONDBLCLK:
		FullscreenHandler(hWnd, VK_RETURN); // emulate enter hit
		break;
	case WM_KEYDOWN:
		FullscreenHandler(hWnd, wParam);
		break;
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		DragHandler(hWnd, dreStart);
		ResizeHandler(hWnd, dreStart);
		break;
	case WM_LBUTTONUP:
		ReleaseCapture();
		DragHandler(hWnd, dreStop);
		ResizeHandler(hWnd, dreStop);
		break;
	case WM_MOUSEMOVE:
		DragHandler(hWnd, dreMove);
		ResizeHandler(hWnd, dreMove);
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
