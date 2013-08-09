// Hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include "Hook.h"

//HWND g_hWnd = NULL;
HOOK_API bool g_bRecord = false;
//HOOK_API std::vector<InputRecord> g_vRecordList;
HOOK_API  DWORD g_dwStartRecordTime = 0;

#pragma data_seg(".Segment")
HWND g_hWnd = NULL;
UINT UWM_KEYDOWN;
UINT UWM_MOUSEDOWN;
UINT UWM_MOUSEUP;
UINT UWM_MOUSEMOVE;
#pragma data_seg()
#pragma comment(linker, "/section:.Segment,rws")

#define UWM_KEYDOWN_MSG ("UWM_KEYDOWN_MSG")
#define UWM_MOUSEDOWN_MSG ("UWM_MOUSEDOWN_MSG")
#define UWM_MOUSEUP_MSG ("UWM_MOUSEUP_MSG")
#define UWM_MOUSEMOVE_MSG ("UWM_MOUSEMOVE_MSG")

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		UWM_KEYDOWN = ::RegisterWindowMessageA(UWM_KEYDOWN_MSG);
		UWM_MOUSEDOWN = ::RegisterWindowMessageA(UWM_MOUSEDOWN_MSG);
		UWM_MOUSEUP = ::RegisterWindowMessageA(UWM_MOUSEUP_MSG);
		UWM_MOUSEMOVE = ::RegisterWindowMessageA(UWM_MOUSEMOVE_MSG);
	case DLL_THREAD_ATTACH:
		//if (!g_fp)
		//	fopen_s(&g_fp,"test.txt","w");
		break;
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		//if (g_fp)
		//	fclose(g_fp);
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

// This is an example of an exported function.
HOOK_API LRESULT HookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	/*char buf[100];
	sprintf_s(buf,"message:%d\n",wParam);
	OutputDebugStringA(buf);*/
	if (nCode < 0)
		return CallNextHookEx(NULL,nCode,wParam,lParam);
	MOUSEHOOKSTRUCT* pMHS = (MOUSEHOOKSTRUCT*)lParam;
	/*char buf[100];
	sprintf(buf,"[hook]message:%x\n",wParam);
	OutputDebugStringA(buf);*/
	if (wParam == WM_LBUTTONDOWN || wParam == WM_NCLBUTTONDOWN)
	{
		char buf[100];
		int k = 0;
		/*char filename[100] = "test";
		srand(GetCurrentThreadId());
		char randomNumber[10];
		sprintf(randomNumber,"%d.txt",rand()%10000);
		strncat(filename,randomNumber,8);
		FILE* fp = NULL;
		fp = fopen(filename,"w");
		int k =	fprintf(fp,"click in %d %d\n",pMHS->pt.x,pMHS->pt.y);*/
		//fflush(g_fp);
		
		//sprintf(buf,"fp:%d thread id:%d k:%d\n",fp,GetCurrentThreadId(),k);
		//OutputDebugStringA(buf);

		HDC hdc = GetDC(pMHS->hwnd);
		Graphics graphics(hdc);
		//sprintf(buf,"mouse win hwnd:%x\n",pMHS->hwnd);
		//OutputDebugStringA(buf);
#if 1
		// Create a SolidBrush object.
		SolidBrush blackBrush(Color(255, (k>0) ? 0 : 255, 0, 0));
		// Define the rectangle.
		POINT wPoint = pMHS->pt;
		MapWindowPoints(NULL,pMHS->hwnd,&wPoint,1);
		int x = wPoint.x;
		int y = wPoint.y;

		int width = 10;
		int height = 10;

		// Fill the rectangle.
		Status st = graphics.FillRectangle(&blackBrush, x, y, width, height);
#else
		// Create a Pen object.
		Pen blackPen(Color(255, 0, 0, 0), 5);

		// Define the rectangle.
		POINT wPoint = pMHS->pt;
		MapWindowPoints(NULL,pMHS->hwnd,&wPoint,1);
		RECT winRECT;
		GetClientRect(pMHS->hwnd,&winRECT);
		int x = 0;//wPoint.x;
		int y = 0;//wPoint.y;
		int width = winRECT.right-winRECT.left;
		int height = winRECT.bottom-winRECT.top;

		// Draw the rectangle.
		graphics.DrawRectangle(&blackPen, x, y, width, height);
#endif
		ReleaseDC(pMHS->hwnd, hdc);

		//char buf[100];
		sprintf(buf,"push mouse %d %d\n",pMHS->pt.x,pMHS->pt.y);
		OutputDebugStringA(buf);
		//if (g_bRecord)
		{
			/*InputRecord r;
			r.bMouse = true;
			r.dwTimeStamp = timeGetTime() - g_dwStartRecordTime;
			r.MouseRecord.x = pMHS->pt.x;
			r.MouseRecord.y = pMHS->pt.y;
			g_vRecordList.push_back(r);*/
			SendMessage( g_hWnd, UWM_MOUSEDOWN, (pMHS->pt.y << 16) + pMHS->pt.x, timeGetTime());
		}
		//fclose(fp);
	}
	else if (wParam == WM_LBUTTONUP || wParam == WM_NCLBUTTONUP)
	{
		SendMessage( g_hWnd, UWM_MOUSEUP, (pMHS->pt.y << 16) + pMHS->pt.x, timeGetTime());
	}
	else if (wParam == WM_MOUSEMOVE)
	{
		SendMessage( g_hWnd, UWM_MOUSEMOVE, (pMHS->pt.y << 16) + pMHS->pt.x, timeGetTime());
	}

	CallNextHookEx(NULL,nCode,wParam,lParam);
	return 0;
}

HOOK_API LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	int mask31 = 0x80000000;
	char buf[100];
	//sprintf(buf,"push some key. nCode:%d wParam:%d 31:%d\n",nCode,wParam,(lParam & mask31));
	//OutputDebugStringA(buf);
	if (nCode < 0)
		return CallNextHookEx(NULL,nCode,wParam,lParam);

	if (((lParam & mask31) == 0) /*&& (wParam != VK_F1)*/) // 1. press down  2. do not record F1
	{
		if (wParam == VK_F2)
			OutputDebugStringA("Hook catch F2 !!!\n");
		//sprintf(buf,"key:%c\n",wParam);
		//OutputDebugStringA(buf);
		//if (g_bRecord)
		{
			sprintf(buf,"!!push key:%c 31:%d hWnd:%x\n",wParam, lParam & mask31,g_hWnd);
			OutputDebugStringA(buf);
			/*InputRecord r;
			r.bMouse = false;
			r.dwTimeStamp = timeGetTime() - g_dwStartRecordTime;
			r.KeyRecord.keyCode = wParam;
			g_vRecordList.push_back(r);*/
			SendMessage( g_hWnd, UWM_KEYDOWN, wParam, timeGetTime());
		}
		/*else
		{
			sprintf(buf,"not record??:%d\n",g_bRecord);
			OutputDebugStringA(buf);
		}*/
	}
	else
	{
		sprintf(buf,"cannot push key:%c 31:%d\n",wParam, lParam & mask31);
		OutputDebugStringA(buf);
	}
	CallNextHookEx(NULL,nCode,wParam,lParam);
	return 0;
}

// This is the constructor of a class that has been exported.
// see Hook.h for the class definition
CHook::CHook()
{
	return;
}

HOOK_API void StartRecord(bool bTrue)
{
	g_bRecord = bTrue;
	if (g_bRecord)
	{
		//OutputDebugStringA("clear record list!\n");
		//g_vRecordList.clear();
		g_dwStartRecordTime = timeGetTime();
	}
}

/*HOOK_API void GetRecordList(std::vector<InputRecord>* pList)
{
	if (pList)
	{
		*pList = g_vRecordList;
	}
}*/

HOOK_API void SetWindowHandle(HWND hWnd)
{
	g_hWnd = hWnd;
}