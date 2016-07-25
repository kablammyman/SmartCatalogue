#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#include "DatabaseController.h"

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:

WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

												// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// Forward declarations of functions included in this code module:
//all of the code will be in seprate cpp files to keep things neat
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK    SQLQuereyBox(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	selectOrDeleteQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HWND CreateNewMDIChild(HWND hMDIClient);
BOOL SetUpMDIChildWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void createSelectOrDeleteBox(HWND hWnd);


HWND g_hMDIDBView;
HWND g_hMDIFileView;
HWND g_hMainWindow;
HINSTANCE hInst;
string szWindowClass;
string szChildWindowClassDB;
string szChildWindowClassDir;
vector<string> dontDeleteList;

vector<string> curFileWindowData;



	DatabaseController dbCtrlr;

	int windowWidth;
	int windowHeight;
	int buttonW, buttonH;
	int baseX, baseY;

	int winWidth;
	int winHeight;

	int dbWindowX, dbWindowY, fileWindowX, fileWindowY;
	string dir;
	string mainWorkingPath;

	string ignorePattern;
	string curDBCommand;

	void initVars();

	void calcWindowSize();

	void setMainWindow(HWND hwnd);

	void setInstance(HINSTANCE i);
	HINSTANCE getInstance();

	const char *getClassName();

	HWND getMainWindowHandle();
	void setMainWindowHandle(HWND hwnd);

	HWND getDBViewHandle();
	void setDBViewHandle(HWND hwnd);

	HWND getFileViewHandle();
	void setFileViewHandle(HWND hwnd);


	void sendDataToEditWindow(HWND hwnd, string data);
	void sendDataToEditWindow(HWND hwnd, vector<string> &data);

