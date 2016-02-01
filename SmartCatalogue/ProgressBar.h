#pragma once
#include "windows.h"

class ProgressBar
{
private:
	int cyVScroll;  // Height of scroll bar arrow.
	HWND hwndPB;    // Handle of progress bar.
	HWND parent;    // Handle of progress bar.

	int x; 
	int y; 
	int size;
	bool isMarquee;
	unsigned int range;
	long long  percentDone = 0;

public:
	ProgressBar();
	ProgressBar(HWND hwndParent, int _x, int _y, int s, unsigned int r,bool m = false);
	void init(HWND hwndParent, int _x, int _y, int s, unsigned int r, bool m = false);
	~ProgressBar();
	void updateProgressBar(int curAmt);
	void setRange(unsigned int r) {range = r;}
};