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
	unsigned int range;
	float newProgress = 0;
	float percentDone = 0;

public:
	ProgressBar(HWND hwndParent, int _x, int _y, int s, unsigned int r);
	~ProgressBar();
	void updateProgressBar(int curAmt);
};