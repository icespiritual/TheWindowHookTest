// Hook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include "Hook.h"

//FILE* g_fp = NULL;

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
	if (wParam == WM_LBUTTONDOWN)
	{
		MOUSEHOOKSTRUCT* pMHS = (MOUSEHOOKSTRUCT*)lParam;

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
		OutputDebugStringA(buf);

		HDC hdc = GetDC(pMHS->hwnd);
		Graphics graphics(hdc);
		sprintf(buf,"mouse win hwnd:%x\n",pMHS->hwnd);
		OutputDebugStringA(buf);
#if 0
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
		//fclose(fp);
	}
	CallNextHookEx(NULL,nCode,wParam,lParam);
	return 0;
}

HOOK_API LRESULT KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(NULL,nCode,wParam,lParam);

	int mask31 = 0x80000000;
	char buf[100];
	if ((lParam & mask31) == 0) // press down
	{
		sprintf(buf,"key:%c 31:%d lpa:%x\n",wParam, lParam & mask31,lParam);
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
