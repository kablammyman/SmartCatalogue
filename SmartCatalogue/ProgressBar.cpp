#include "stdafx.h"
#include "ProgressBar.h"

#include "CommCtrl.h"
ProgressBar::ProgressBar()
{
	x = 0;
	y = 0;
	range = 100;
	size = 100;
	parent = NULL;
}

ProgressBar::ProgressBar(HWND hwndParent, int _x, int _y, int s, unsigned int r, bool m)
{
	init(hwndParent, _x, _y, s, r, isMarquee);
}

void ProgressBar::init(HWND hwndParent, int _x, int _y, int s, unsigned int r, bool m)
{
	x = _x;
	y = _y;
	range = r;
	size = s;
	parent = hwndParent;
	isMarquee = m;

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
	if (!isMarquee)
	{
		hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, x, y, size, cyVScroll, hwndParent, (HMENU)0, GetModuleHandle(NULL), NULL);
		SendMessage(hwndPB, PBM_SETRANGE, 0, MAKELPARAM(0, size));
		SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)1, 0);
		//SendMessage(hwndPB, PBM_SETSTEP, (WPARAM)(size / 100), 0);
	}
	else
	{
		hwndPB = CreateWindowEx(0, PROGRESS_CLASS, (LPTSTR)NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH | PBS_MARQUEE, x, y, size, cyVScroll, hwndParent, (HMENU)0, GetModuleHandle(NULL), NULL);
		SendMessage(hwndPB, PBM_SETMARQUEE, (WPARAM)1, (LPARAM)NULL);
	}

	


	percentDone = 0;
}

ProgressBar::~ProgressBar()
{
	SendMessage(hwndPB, WM_DESTROY, 0, 0);
	DestroyWindow(hwndPB);
}

void ProgressBar::updateProgressBar(int curAmt)
{
	if (!isMarquee)
	{
		percentDone = (curAmt * 100) / range;

		percentDone *= (size / 100);
		SendMessage(hwndPB, PBM_SETPOS, percentDone, 0);
	}
	else
	{
		SendMessage(hwndPB, PBM_SETMARQUEE, (WPARAM)1, (LPARAM)NULL);
	}
}