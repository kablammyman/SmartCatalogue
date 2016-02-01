#pragma once

#include "resource.h"
#include <windows.h>
#include "CommCtrl.h"
#include "DatabaseController.h"

#define MAX_LOADSTRING 100

DatabaseController dbCtrlr;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND mainWindowHandle;


HWND hwndTree;
HTREEITEM Selected;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
BOOL InitTreeViewItems(HWND hwndTV);
HTREEITEM AddItemToTree(HWND hwndTV, wchar_t* lpszItem, int nLevel);
void lilMenu(HWND handle, int x, int y);
void findAllTheDupes(HWND hwndTV);
void addNewItemToTree(HWND hwndTV, string curMD5, string quereyOutput);