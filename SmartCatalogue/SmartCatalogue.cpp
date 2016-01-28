// imageView.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SmartCatalogue.h"
#include "MainApp.h"

#include <utility>      // std::pair
#include <string>

#include <iostream>
#include <fstream>
#include <sstream>

HWND g_hMDIClient = NULL;

using namespace std;

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	//LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	//LoadString(hInstance, IDC_IMAGEVIEW, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!SetUpMDIChildWindowClass(hInstance))
		return 0;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_IMAGEVIEW));

	

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		//lil child windows
		if (!TranslateMDISysAccel(g_hMDIClient, &msg) && !TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}		
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= 0;//CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_IMAGEVIEW));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_IMAGEVIEW);
	wcex.lpszClassName	= MainApp::Instance()->getClassName();
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	
   MainApp::Instance()->setInstance(hInstance);// Store instance handle in our global variable
   
	HWND hWnd = CreateWindow(MainApp::Instance()->getClassName(), "image view", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, MainApp::Instance()->winWidth,MainApp::Instance()->winHeight, NULL, NULL, hInstance, NULL);
	MainApp::Instance()->setMainWindow(hWnd);
	MainApp::Instance()->calcWindowSize();

	// Create Status bar
	HWND hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,hWnd, (HMENU)IDC_MAIN_STATUS, GetModuleHandle(NULL), NULL);
	SendMessage(hStatus, SB_SETPARTS, 1000, (LPARAM)1000);
	SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM)"Hi there :)");
	
	// Create Toolbar
	HWND hTool = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, hWnd, (HMENU)IDC_MAIN_TOOL, GetModuleHandle(NULL), NULL);
	if (hTool == NULL)
		MessageBox(hWnd, "Could not create tool bar.", "Error", MB_OK | MB_ICONERROR);

	// Send the TB_BUTTONSTRUCTSIZE message, which is required forbackward compatibility.

	//HIMAGELIST hImageList = ImageList_LoadBitmap(hInstance, MAKEINTRESOURCEW(IDB_MADE_SHOT_PIC), 16, 0, RGB(255, 0, 255));
	//ImageList_Add(hImageList, LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MISSED_SHOT_PIC)), NULL);
	//ImageList_Add(hImageList, LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_MISSED_SHOT_PIC)), NULL);

	//SendMessage(hWndToolbar, TB_SETIMAGELIST, (WPARAM)ID_IL_STANDARD, (LPARAM)hImageList);
	//SendMessage(hWndToolbar, (UINT)TB_SETHOTIMAGELIST, 0, (LPARAM)hHotImageList);
	//SendMessage(hWndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);


	

	TBBUTTON tbb[5];
	TBADDBITMAP tbab;


	SendMessage(hTool, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	tbab.hInst = HINST_COMMCTRL;
	tbab.nID = IDB_STD_SMALL_COLOR;

	
	SendMessage(hTool, TB_ADDBITMAP, IDB_MISSED_SHOT_PIC, (LPARAM)&tbab);

	ZeroMemory(tbb, sizeof(tbb));
	tbb[0].iBitmap = STD_FILENEW;
	tbb[0].fsState = TBSTATE_ENABLED;
	tbb[0].fsStyle = TBSTYLE_BUTTON;
	tbb[0].idCommand = ID_FILE_DB_NEW;

	tbb[1].iBitmap = STD_FILEOPEN;
	tbb[1].fsState = TBSTATE_ENABLED;
	tbb[1].fsStyle = TBSTYLE_BUTTON;
	tbb[1].idCommand = ID_FILE_DB_OPEN;

	tbb[2].iBitmap = STD_FILESAVE;
	tbb[2].fsState = TBSTATE_ENABLED;
	tbb[2].fsStyle = TBSTYLE_BUTTON;
	tbb[2].idCommand = ID_FILE_CONFIG_SAVE;

	tbb[3].iBitmap = IDB_MISSED_SHOT_PIC;
	tbb[3].fsState = TBSTATE_ENABLED;
	tbb[3].fsStyle = TBSTYLE_BUTTON;
	tbb[3].idCommand = ID_SELECT_OR_DELETE_BUTTON;

	tbb[4].iBitmap = IDB_MADE_SHOT_PIC;
	tbb[4].fsState = TBSTATE_ENABLED;
	tbb[4].fsStyle = TBSTYLE_BUTTON;
	tbb[4].idCommand = ID_TEST_BUTTON;


	SendMessage(hTool, TB_AUTOSIZE, 0, 0);
	ShowWindow(hTool, SW_SHOW);
	SendMessage(hTool, TB_ADDBUTTONS, sizeof(tbb) / sizeof(TBBUTTON), (LPARAM)&tbb);


	/*
	TBBUTTON tbb[3] =
	{
		{ 0,ID_CUT,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 1,ID_COPY,TBSTATE_ENABLED,TBSTYLE_BUTTON },
		{ 2,ID_PASTE,TBSTATE_ENABLED,TBSTYLE_BUTTON },
	};
	*/

	
	HWND dbEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		MainApp::Instance()->dbWindowX, MainApp::Instance()->dbWindowY, MainApp::Instance()->windowWidth/2, MainApp::Instance()->windowHeight/2, hWnd, (HMENU)IDC_CHILD_EDIT, GetModuleHandle(NULL), NULL);
	MainApp::Instance()->setDBViewHandle(dbEdit);
	ShowWindow(dbEdit, SW_HIDE);

	HWND fileEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		MainApp::Instance()->fileWindowX, MainApp::Instance()->fileWindowY, MainApp::Instance()->windowWidth/2, MainApp::Instance()->windowHeight/2, hWnd, (HMENU)IDC_CHILD_EDIT, GetModuleHandle(NULL), NULL);
	MainApp::Instance()->setFileViewHandle(fileEdit);
	ShowWindow(fileEdit, SW_HIDE);

	//CreateWindowA("BUTTON", "Add dirs to DB",  WS_VISIBLE | WS_CHILD,MainApp::Instance()->baseX, MainApp::Instance()->baseY,   MainApp::Instance()->buttonW,MainApp::Instance()->buttonH,MainApp::Instance()->getMainWindowHandle(),(HMENU)(BTN_ADD_DIR_TO_DB),NULL,0);
	
	//CreateWindowA("BUTTON", "Delete?",  WS_VISIBLE | WS_CHILD,MainApp::Instance()->fileWindowX, MainApp::Instance()->baseY,    MainApp::Instance()->buttonW,MainApp::Instance()->buttonH,MainApp::Instance()->getMainWindowHandle(),(HMENU)(BTN_DELETE_FROM_FILE_PATH_LIST),NULL,0);
	
	

   if (!hWnd)
   {
      return FALSE;
   }


   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

  // opencvTestFn(hWnd, "C:\\Users\\Victor\\Desktop\\hanging shelf-no posts.jpg");
   return TRUE;
}
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;



	switch (message)
	{
	case WM_CREATE:
		//lil child windows  Create MDI Client
		CLIENTCREATESTRUCT ccs;
		// Find window menu where children will be listed
		ccs.hWindowMenu = GetSubMenu(GetMenu(hWnd), 2);
		ccs.idFirstChild = ID_MDI_FIRSTCHILD;

		g_hMDIClient = CreateWindowEx(WS_EX_CLIENTEDGE, "MDICLIENT", NULL, WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			hWnd, (HMENU)IDC_MAIN_MDI, GetModuleHandle(NULL), (LPVOID)&ccs);

		ShowWindow(g_hMDIClient, SW_SHOW);
	break;
	case WM_COMMAND:
		//do file open
		if(LOWORD(wParam) == BTN_ADD_DIR_TO_DB)
		{
			if(!MainApp::Instance()->dbCtrlr.isDBOpen())
				 MessageBox(hWnd, "Open a database first!","db not open", MB_OKCANCEL);
			else
			{
				//we need to figure out how to make sure MainApp::Instance()->dir exists
				string output;
				MainApp::Instance()->dbCtrlr.insertNewDataEntry("someTableName", make_pair("some field", "some value"), output);
			}
		}
	
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_FILE_DB_OPEN:
			{
				std::string fileName;
				MainApp::Instance()->DoFileOpen(hWnd,fileName);
				if(!fileName.empty())
				{
					MainApp::Instance()->dbCtrlr.openDatabase(fileName);
				}
				RedrawWindow( hWnd, 0, 0, RDW_INVALIDATE | RDW_NOCHILDREN );	
			}
			break;
		case ID_TEST_BUTTON:
			DialogBox(MainApp::Instance()->getInstance(), MAKEINTRESOURCE(IDD_MAIN_SELECTOR), hWnd, DlgProc);
			break;
		case ID_SELECT_OR_DELETE_BUTTON:
			createSelectOrDeleteBox(hWnd);
			break;

		case ID_FILE_DB_NEW:
			{
				MainApp::Instance()->callCreateNewDatabase();
				//CreateNewMDIChild(g_hMDIClient);
			}
			break;
		case ID_FILE_CONFIG_SAVE:
			{
				std::string fileName;
				MainApp::Instance()->DoFileSave(hWnd,fileName); 
			}
			break;
		case IDM_DATABASE_ADD_ENTRIES:
			DialogBox(MainApp::Instance()->getInstance(), MAKEINTRESOURCE(IDD_ADDBOX), hWnd, AddRecordBox);
			break;
		case IDM_DATABASE_RUN_SQL_COMMAND:
			if(MainApp::Instance()->dbCtrlr.isDBOpen())
				DialogBox(MainApp::Instance()->getInstance(), MAKEINTRESOURCE(IDD_SQLBOX), hWnd, SQLQuereyBox);
			else
				 MessageBox(hWnd, "Open a database first!","db not open", MB_OKCANCEL);
			
			break;
		case IDM_ABOUT:
			DialogBox(MainApp::Instance()->getInstance(), MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);

		TextOutA(hdc,MainApp::Instance()->dbWindowX,MainApp::Instance()->dbWindowY-20,"Database: ",(int)strlen("Database: "));
		TextOutA(hdc,MainApp::Instance()->dbWindowX+70,MainApp::Instance()->dbWindowY-20,MainApp::Instance()->dbCtrlr.getDBName().c_str(), (int)MainApp::Instance()->dbCtrlr.getDBName().size());
		
		TextOutA(hdc,MainApp::Instance()->fileWindowX,MainApp::Instance()->fileWindowY-20,"File Path: ", (int)strlen("File Path: "));
		TextOutA(hdc,MainApp::Instance()->fileWindowX+70,MainApp::Instance()->fileWindowY-20,MainApp::Instance()->dir.c_str(), (int)MainApp::Instance()->dir.size());

		
		EndPaint(hWnd, &ps);
		break;
	case WM_SIZE:
	{
		HWND hTool;
		RECT rcTool;
		int iToolHeight;

		HWND hStatus;
		RECT rcStatus;
		int iStatusHeight;

		HWND hMDI;
		int iMDIHeight;
		RECT rcClient;

		// Size toolbar and get height

		hTool = GetDlgItem(hWnd, IDC_MAIN_TOOL);
		SendMessage(hTool, TB_AUTOSIZE, 0, 0);

		GetWindowRect(hTool, &rcTool);
		iToolHeight = rcTool.bottom - rcTool.top;

		// Size status bar and get height

		hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
		SendMessage(hStatus, WM_SIZE, 0, 0);

		GetWindowRect(hStatus, &rcStatus);
		iStatusHeight = rcStatus.bottom - rcStatus.top;

		// Calculate remaining height and size edit

		GetClientRect(hWnd, &rcClient);

		iMDIHeight = rcClient.bottom - iToolHeight - iStatusHeight;

		hMDI = GetDlgItem(hWnd, IDC_MAIN_MDI);
		SetWindowPos(hMDI, NULL, 0, iToolHeight, rcClient.right, iMDIHeight, SWP_NOZORDER);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}




