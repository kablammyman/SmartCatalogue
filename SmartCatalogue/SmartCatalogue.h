#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include "MainApp.h"
#include "resource.h"


// Forward declarations of functions included in this code module:
//all of the code will be in seprate cpp files to keep things neat
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AddRecordBox(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    SQLQuereyBox(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	selectOrDeleteQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK QuickFind(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam);
HWND CreateNewMDIChild(HWND hMDIClient);
BOOL SetUpMDIChildWindowClass(HINSTANCE hInstance);
LRESULT CALLBACK MDIChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void createSelectOrDeleteBox(HWND hWnd);
#endif