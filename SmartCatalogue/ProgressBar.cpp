#include "stdafx.h"
#include "ProgressBar.h"

#include "CommCtrl.h"

ProgressBar::ProgressBar(HWND hwndParent, int _x, int _y, int s, unsigned int r)
{
	x = _x;
	y = _y;
	range = r;
	size = s;
	parent = hwndParent;

	INITCOMMONCONTROLSEX InitCtrlEx;
	InitCtrlEx.dwSize = sizeof(INITCOMMONCONTROLSEX);
	InitCtrlEx.dwICC = ICC_LINK_CLASS, ICC_PROGRESS_CLASS;
	InitCommonControlsEx(&InitCtrlEx);

	if (size == 0)
	{
		RECT rcClient;  // Client area of parent window.
		GetClientRect(hwndParent, &rcClient);

		size = rcClient.right - rcClient.left;
	}

	cyVScroll = GetSystemMetrics(SM_CYVSCROLL);

	//PBS_SMOOTH without this, thw bar is a bunch of blocks
	//GetModuleHandle(NULL); gets the hinstance...that way i wont need a global var in here
	hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, x, y, size, cyVScroll, hwndParent, (HMENU)0, GetModuleHandle(NULL), NULL);

	
	SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, size));
	SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)1, 0);

	//SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)(size / 100), 0);


	percentDone = 0;
}

ProgressBar::~ProgressBar()
{
	DestroyWindow(hwndPB);
}

void ProgressBar::updateProgressBar(int curAmt)
{
	percentDone = (curAmt * 100) / range;
	
	percentDone *= (size / 100);
	SendMessage(hwndPB, PBM_SETPOS, percentDone, 0);
}