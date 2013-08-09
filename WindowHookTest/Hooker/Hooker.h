#pragma once

#include "resource.h"

typedef struct _InputRecord
{
	bool bMouse; // 1: mouse 0: keyboard
	union{
		struct {
			unsigned short x;
			unsigned short y;
		}MouseRecord;
		struct {
			int	keyCode;
		}KeyRecord;
	};
	DWORD dwTimeStamp;
	bool bPressDown;
	bool bMoving;
}InputRecord;