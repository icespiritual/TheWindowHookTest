// Hooker.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <stdio.h>
#include <vector>
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
bool g_bRecording = false;
std::vector<InputRecord> g_vRecordList;
DWORD g_dwStartRecordTime = 0;
bool g_bHooking = false;

#define UWM_KEYDOWN_MSG ("UWM_KEYDOWN_MSG")
static UINT UWM_KEYDOWN = ::RegisterWindowMessageA(UWM_KEYDOWN_MSG);
#define UWM_MOUSEDOWN_MSG ("UWM_MOUSEDOWN_MSG")
static UINT UWM_MOUSEDOWN = ::RegisterWindowMessageA(UWM_MOUSEDOWN_MSG);
#define UWM_MOUSEUP_MSG ("UWM_MOUSEUP_MSG")
static UINT UWM_MOUSEUP = ::RegisterWindowMessageA(UWM_MOUSEUP_MSG);
#define UWM_MOUSEMOVE_MSG ("UWM_MOUSEMOVE_MSG")
static UINT UWM_MOUSEMOVE = ::RegisterWindowMessageA(UWM_MOUSEMOVE_MSG);

typedef LRESULT (WINAPI* fHookProc)(int nCode, WPARAM wParam, LPARAM lParam);
typedef void (*SendInputRecord)(InputRecord* pRecord);
typedef void (*SetCallback)(SendInputRecord func);
typedef void (*fStartRecord)(bool bTrue);
typedef void (*fGetList)(std::vector<InputRecord>* pList);
typedef void (*SetWindowHandle)(HWND hWnd);

void StopHook();

void fSendInputRecord(InputRecord* pRecord)
{
	/*char buf[100];
	sprintf(buf,"WOW!! hook:%x\n",g_hHook);
	OutputDebugStringA(buf);*/
	/*if (g_bRecording && pRecord)
	{
		pRecord->dwTimeStamp -= g_dwStartRecordTime;
		g_vRecordList.push_back(*pRecord);
		OutputDebugStringA("record!\n");
	}*/
	OutputDebugStringA("try record!\n");
}

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

void StartHook(HWND hWnd)
{
	/////////////// start hook //////////////////////////////
	if (!g_hHookMod)
	{
		g_hHookMod = LoadLibraryA(".\\Hook.dll");
		if (!g_hHookMod)
			g_hHookMod = LoadLibraryA(".\\\debug\\Hook.dll");
	}

	if (g_hHookMod && !g_hHook)
	{
		OutputDebugStringA("Load Hook!\n");
		fHookProc fpHookProc = NULL;
		fHookProc fpHookProcKey = NULL;
		fpHookProc = (fHookProc)GetProcAddress(g_hHookMod,"HookProc");
		if (fpHookProc)
		{
			OutputDebugStringA("find proc!\n");
			if (g_hHook)
				UnhookWindowsHookEx(g_hHook);
			if (g_hHookKey)
				UnhookWindowsHookEx(g_hHookKey);
			g_hHook = SetWindowsHookExA(WH_MOUSE,fpHookProc,g_hHookMod,0);
			fpHookProcKey = (fHookProc)GetProcAddress(g_hHookMod,"KeyboardProc");
			OutputDebugStringA("hook leyboard\n");
			g_hHookKey = SetWindowsHookExA(WH_KEYBOARD,fpHookProcKey,g_hHookMod,0);
			SetCallback fpCallback = (SetCallback)GetProcAddress(g_hHookMod,"SetCallback");
			if (fpCallback)
				fpCallback(&fSendInputRecord);

			SetWindowHandle fSetHandle = (SetWindowHandle)GetProcAddress(g_hHookMod,"SetWindowHandle");
			if (fSetHandle)
			{
				char buf[100];
				sprintf(buf,"set win handle:%x\n",hWnd);
				OutputDebugStringA(buf);
				fSetHandle(hWnd);
			}

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
	g_bHooking = true;
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

	/*char buf[100];
	sprintf(buf,"message:%x\n",message);
	OutputDebugStringA(buf);*/

	//if (g_bRecording)
	{
		if (message == UWM_KEYDOWN)
		{
			if (wParam == VK_F1)
			{
				OutputDebugStringA("Hook press F1!\n");
				g_bRecording = !g_bRecording;
				if (g_bRecording)
				{
					//StartHook(hWnd);
					g_vRecordList.clear(); // reset record list
					g_dwStartRecordTime = timeGetTime();
					OutputDebugStringA("hook start record!\n");
				}
				else
				{
					OutputDebugStringA("hook stop record!\n");
				}
				if (g_hHookMod)
				{
					fStartRecord fpStartRecord = (fStartRecord)GetProcAddress(g_hHookMod,"StartRecord");
					if (fpStartRecord)
						fpStartRecord(g_bRecording);
				}
			}
			else if (wParam == VK_F2)
			{
				OutputDebugStringA("Hook press F2!\n");
				if (g_bRecording)
				{
					OutputDebugStringA("Still recording! cannot play.\n");
					//return;
					//break;
				}
				StopHook();
				/*if (g_hHookMod)
				{
					fGetList fpGetList = (fGetList)GetProcAddress(g_hHookMod,"GetRecordList");
					if (fpGetList)
					{
						OutputDebugStringA("get record list\n");
						fpGetList(&g_vRecordList);
					}
				}*/
				char buf[100];
				sprintf(buf,"Start play! input count:%d.\n",g_vRecordList.size());
				OutputDebugStringA(buf);
				for (int i=0;i<g_vRecordList.size();i++)
				{
					if (i == 0)
					{
						Sleep(g_vRecordList[i].dwTimeStamp);
					}
					else
					{
						// skip repeat record, will happen if target app has two windows to receive mouse/keyboard message
						if (0 == memcmp(&g_vRecordList[i],&g_vRecordList[i-1],sizeof(InputRecord)))
							continue;
						Sleep(g_vRecordList[i].dwTimeStamp - g_vRecordList[i-1].dwTimeStamp);
					}
					if (g_vRecordList[i].bMouse)
					{
						char buf[100];
						sprintf(buf,"play mouse %d %d\n",g_vRecordList[i].MouseRecord.x,g_vRecordList[i].MouseRecord.y);
						OutputDebugStringA(buf);
						INPUT input;
						input.type = INPUT_MOUSE;
						input.mi.dx = 65536*g_vRecordList[i].MouseRecord.x/1920;
						input.mi.dy = 65536*g_vRecordList[i].MouseRecord.y/1080;
						input.mi.mouseData = 0;
						input.mi.time = 0;

						if (g_vRecordList[i].bMoving)
						{
							OutputDebugStringA("mouse move\n");
							input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE /*| MOUSEEVENTF_LEFTDOWN*/;
						}
						else if (g_vRecordList[i].bPressDown)
						{
							sprintf(buf,"mouse down time:%d\n",g_vRecordList[i].dwTimeStamp);
							OutputDebugStringA(buf);
							input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						}
						else
						{
							sprintf(buf,"mouse up time:%d\n",g_vRecordList[i].dwTimeStamp);
							OutputDebugStringA(buf);
							input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
						}
						SendInput(1,&input,sizeof(INPUT));
						//OutputDebugStringA("AA\n");
						/*Sleep(10);
						input.mi.dx = 0;
						input.mi.dy = 0;
						input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
						SendInput(1,&input,sizeof(INPUT));
						//OutputDebugStringA("AA\n");
						Sleep(10);
						input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
						SendInput(1,&input,sizeof(INPUT));*/
						//OutputDebugStringA("AA\n");
					}
					else
					{
						sprintf(buf,"keyboard time:%d\n",g_vRecordList[i].dwTimeStamp);
						OutputDebugStringA(buf);
						//OutputDebugStringA("Keyboard\n");
						INPUT input;
						input.type = INPUT_KEYBOARD; 
						input.ki.wVk = g_vRecordList[i].KeyRecord.keyCode;//VkKeyScan(L's');
						input.ki.wScan = MapVirtualKey(g_vRecordList[i].KeyRecord.keyCode, 0); 
						input.ki.dwFlags = 0;
						SendInput(1, &input, sizeof(INPUT));
						Sleep(10);
						//input.type = INPUT_KEYBOARD; 
						//input.ki.wVk = VkKeyScan(L's');
						//input.ki.wScan = MapVirtualKey(g_vRecordList[i].KeyRecord.keyCode, 0);
						input.ki.dwFlags = KEYEVENTF_KEYUP;
						SendInput(1, &input, sizeof(INPUT));
					}
				}
			} // f2
			if (g_bRecording)
			{
				InputRecord r;
				r.bMouse = false;
				r.dwTimeStamp = timeGetTime() - g_dwStartRecordTime;
				r.KeyRecord.keyCode = wParam;
				g_vRecordList.push_back(r);
				OutputDebugStringA("Really push a key Input.\n");
			}
		}
		else if (g_bRecording)
		{
			if (message == UWM_MOUSEDOWN)
			{
				InputRecord r;
				r.bMouse = true;
				r.dwTimeStamp = timeGetTime() - g_dwStartRecordTime;
				r.MouseRecord.x = wParam & 0x0000FFFF;
				r.MouseRecord.y = wParam >> 16;
				r.bPressDown = true;
				r.bMoving = false;
				g_vRecordList.push_back(r);
				OutputDebugStringA("Really send a mouse Input.\n");
			}
			else if (message == UWM_MOUSEMOVE)
			{
				InputRecord r;
				r.bMouse = true;
				r.dwTimeStamp = timeGetTime() - g_dwStartRecordTime;
				r.MouseRecord.x = wParam & 0x0000FFFF;
				r.MouseRecord.y = wParam >> 16;
				r.bPressDown = true;
				r.bMoving = true;
				g_vRecordList.push_back(r);
			}
			else if (message == UWM_MOUSEUP)
			{
				InputRecord r;
				r.bMouse = true;
				r.dwTimeStamp = timeGetTime() - g_dwStartRecordTime;
				r.MouseRecord.x = wParam & 0x0000FFFF;
				r.MouseRecord.y = wParam >> 16;
				r.bPressDown = false;
				r.bMoving = false;
				g_vRecordList.push_back(r);
			}
		}
	}
	switch (message)
	{
	case WM_KEYDOWN:
		/*if (wParam == 'c' || wParam == 'C')
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
		else if (wParam == 'm' || wParam == 'M')
		{
		}*/
		if (wParam == VK_F1) // turn on/off record
		{
			if (!g_bHooking)
			{
				g_bRecording = !g_bRecording;
				if (g_bRecording)
				{
					StartHook(hWnd);
					g_vRecordList.clear(); // reset record list
					g_dwStartRecordTime = timeGetTime();
					OutputDebugStringA("start record!\n");
				}
				else
				{
					//StopHook();
					OutputDebugStringA("stop record!\n");
				}
				if (g_hHookMod)
				{
					fStartRecord fpStartRecord = (fStartRecord)GetProcAddress(g_hHookMod,"StartRecord");
					if (fpStartRecord)
						fpStartRecord(g_bRecording);
				}
			}
		}
#if 0
		else if (wParam == VK_F2) // play recorded input
		{
			StopHook();
			if (g_bRecording)
			{
				OutputDebugStringA("Still recording! cannot play.\n");
				break;
			}
			/*if (g_hHookMod)
			{
				fGetList fpGetList = (fGetList)GetProcAddress(g_hHookMod,"GetRecordList");
				if (fpGetList)
				{
					OutputDebugStringA("get record list\n");
					fpGetList(&g_vRecordList);
				}
			}*/
			char buf[100];
			sprintf(buf,"Start play! input count:%d.\n",g_vRecordList.size());
			OutputDebugStringA(buf);
			for (int i=0;i<g_vRecordList.size();i++)
			{
				if (i == 0)
					Sleep(g_vRecordList[i].dwTimeStamp);
				else
					Sleep(g_vRecordList[i].dwTimeStamp - g_vRecordList[i-1].dwTimeStamp);
				if (g_vRecordList[i].bMouse)
				{
					char buf[100];
					sprintf(buf,"play mouse %d %d\n",g_vRecordList[i].MouseRecord.x,g_vRecordList[i].MouseRecord.y);
					OutputDebugStringA(buf);
					INPUT input;
					input.type = INPUT_MOUSE;
					input.mi.dx = 65536*g_vRecordList[i].MouseRecord.x/1920;
					input.mi.dy = 65536*g_vRecordList[i].MouseRecord.y/1080;
					input.mi.mouseData = 0;
					input.mi.time = 0;

					if (g_vRecordList[i].bMoving)
					{
						OutputDebugStringA("mouse move\n");
						input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE /*| MOUSEEVENTF_LEFTDOWN*/;
					}
					else if (g_vRecordList[i].bPressDown)
					{
						OutputDebugStringA("mouse down\n");
						input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
					}
					else
					{
						OutputDebugStringA("mouse up\n");
						input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
					}
					SendInput(1,&input,sizeof(INPUT));
					//OutputDebugStringA("AA\n");
					/*Sleep(10);
					input.mi.dx = 0;
					input.mi.dy = 0;
					input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
					SendInput(1,&input,sizeof(INPUT));
					//OutputDebugStringA("AA\n");
					Sleep(10);
					input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
					SendInput(1,&input,sizeof(INPUT));*/
					//OutputDebugStringA("AA\n");
				}
				else
				{
					OutputDebugStringA("Keyboard\n");
					INPUT input;
					input.type = INPUT_KEYBOARD; 
					input.ki.wVk = g_vRecordList[i].KeyRecord.keyCode;//VkKeyScan(L's');
					input.ki.wScan = MapVirtualKey(g_vRecordList[i].KeyRecord.keyCode, 0); 
					input.ki.dwFlags = 0;
					SendInput(1, &input, sizeof(INPUT));
					Sleep(10);
					//input.type = INPUT_KEYBOARD; 
					//input.ki.wVk = VkKeyScan(L's');
					//input.ki.wScan = MapVirtualKey(g_vRecordList[i].KeyRecord.keyCode, 0);
					input.ki.dwFlags = KEYEVENTF_KEYUP;
					SendInput(1, &input, sizeof(INPUT));
				}
			}
		}
#endif
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
		OutputDebugStringA("paint.\n");
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

void StopHook()
{
	g_bHooking = false;
	// release hook resource
	BOOL unhooked = UnhookWindowsHookEx(g_hHook);
	g_hHook = 0;
	unhooked = UnhookWindowsHookEx(g_hHookKey);
	g_hHookKey = 0;
}