#pragma once

#include "resource.h"
#include "DatabaseController.h"

#define MAX_LOADSTRING 100

// Global Variables:
extern HINSTANCE mainInst;                                // current instance
extern DatabaseController dbCtrlr;

												// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    SQLQuereyBox(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);

LRESULT NotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

