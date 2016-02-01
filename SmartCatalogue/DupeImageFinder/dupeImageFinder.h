#pragma once

#include "resource.h"
#include <windows.h>
#include "CommCtrl.h"
#include "DatabaseController.h"
#include "ProgressBar.h"

#define MAX_LOADSTRING 100

DatabaseController dbCtrlr;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND mainWindowHandle;
HWND statusText;

HWND hwndTreeView;
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
void init();
void mainLogic();
void finish();
//usewd to time how long it takes to find the dupes
bool readyToSearch = false;
int start_s, stop_s;
vector<DatabaseController::dbDataPair> hashes;
//ProgressBar waitingMarquee;
ProgressBar progress;
int loopIndex = 0;