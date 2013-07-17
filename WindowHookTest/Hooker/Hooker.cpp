// Hooker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdio.h>
#include "Hooker.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

HHOOK g_hHook = NULL;
HHOOK g_hHookKey = NULL;
HMODULE g_hHookMod = NULL;
bool g_bHookMouse = true;
typedef LRESULT (WINAPI* fHookProc)(int nCode, WPARAM wParam, LPARAM lParam);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR           gdiplusToken;
   
    // Initialize GDI+.
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_HOOKER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_HOOKER));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	GdiplusShutdown(gdiplusToken);
	return (int) msg.wParam;
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

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_HOOKER));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_HOOKER);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	bool bMouse = false;

	switch (message)
	{
	case WM_KEYDOWN:
		if (wParam == 'c' || wParam == 'C')
		{
			Sleep(1000);
			INPUT input;
			input.type = INPUT_MOUSE;
			input.mi.dx = 65536*500/1920;
			input.mi.dy = 65536*500/1080;
			input.mi.mouseData = 0;
			input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
			input.mi.time = 0;
			SendInput(1,&input,sizeof(INPUT));
			Sleep(20);
			input.mi.dx = 0;
			input.mi.dy = 0;
			input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
			SendInput(1,&input,sizeof(INPUT));
			Sleep(20);
			input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
			SendInput(1,&input,sizeof(INPUT));
		}
		else
		{
			if (wParam == 'k' || wParam == 'K')
			{
				bMouse = false;
			}
			else if (wParam == 'm' || wParam == 'M')
			{
				bMouse = true;
			}
			else
			{
				break;
			}
			/////////////// start hook //////////////////////////////
			if (!g_hHookMod)
				g_hHookMod = LoadLibraryA(".\\Hook.dll");

			if (g_hHookMod && (!g_hHook || g_bHookMouse != bMouse))
			{
				OutputDebugStringA("Load Hook!\n");
				fHookProc fpHookProc = NULL;
				fHookProc fpHookProcKey = NULL;
				if (bMouse)
					fpHookProc = (fHookProc)GetProcAddress(g_hHookMod,"HookProc");
				else
					fpHookProc = (fHookProc)GetProcAddress(g_hHookMod,"KeyboardProc");
				if (fpHookProc)
				{
					OutputDebugStringA("find proc!\n");
					if (g_hHook)
						UnhookWindowsHookEx(g_hHook);
					g_hHook = SetWindowsHookExA( (bMouse) ? WH_MOUSE : WH_KEYBOARD,fpHookProc,g_hHookMod,0);
					fpHookProcKey = (fHookProc)GetProcAddress(g_hHookMod,"KeyboardProc");
					g_hHookKey = SetWindowsHookExA(WH_KEYBOARD,fpHookProcKey,g_hHookMod,0);
					if (!g_hHook)
					{
						DWORD dwError = GetLastError();
						OutputDebugStringA("set hook fail!\n");
					}
					if (!g_hHookKey)
					{
						DWORD dwError = GetLastError();
						OutputDebugStringA("set hook key fail!\n");
					}
				}
				else
				{
					OutputDebugStringA("cannot find proc!\n");
				}
				g_bHookMouse = bMouse;
			}
			else
			{
				if (!g_hHookMod)
				{
					DWORD dwError = GetLastError();
					char buf[100];
					sprintf_s(buf,"cannot load Hook.dll?? error:%d\n",dwError);
					OutputDebugStringA(buf);
				}
			}
		}
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		{
			Graphics graphics(hdc);

			// Create a SolidBrush object.
			SolidBrush blackBrush(Color(255, 0, 0, 0));
			// Define the rectangle.
		    int x = 0;
		    int y = 0;
		    int width = 100;
		    int height = 100;

		    // Fill the rectangle.
		    Status st = graphics.FillRectangle(&blackBrush, x, y, width, height);
			st = st;
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		{
			// release hook resource
			BOOL unhooked = UnhookWindowsHookEx(g_hHook);
			unhooked = UnhookWindowsHookEx(g_hHookKey);
			FreeLibrary(g_hHookMod);
			PostQuitMessage(0);
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
