// DBLookup.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DBLookup.h"
#include "CFGUtils.h"
#include "CFGHelper.h"
#include "StringUtils.h"
#include <Commctrl.h> //for listview
#include <string>
#include <vector>

using namespace std;

#define NUM_COL 1

WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

HWND CreateListView(HWND hwndParent, vector<string> colNames, int x, int y, int w, int h, int size);
BOOL FillListViewItems(HWND hWndListView, vector<string> items);
//LRESULT NotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HINSTANCE mainInst;                                // current instance
DatabaseController dbCtrlr;
HWND listView;

/*
// structures
typedef struct tagHOUSEINFO
{
	char szAddress[100];
	char szCity[100];
} HOUSEINFO;

HOUSEINFO doubleArray[] =
{
	{ "100 Main Street", "Redmond" },
	{ "523 Pine Lake Road", "Redmond"},
	{ "1212 112th Place SE", "Redmond" },
	{ "22 Lake Washington Blvd", "Bellevue" },
	{ "33542 116th Ave. NE", "Bellevue" },
	{ "64134 Nicholas Lane", "Bellevue" },
	{ "33 Queen Anne Hill", "Seattle" },
	{ "555 SE Fifth St", "Seattle"},
	{ "446 Mariners Way", "Seattle"}
};*/

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
   // UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DBLOOKUP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DBLOOKUP));

    MSG msg;
	wstring temp(lpCmdLine);
	string cmdLine(temp.begin(),temp.end());

	CFGHelper::filePathBase = CFGHelper::SetProgramPath(cmdLine);
	if (!CFGHelper::LoadCFGFile())
	{
		MessageBox(NULL, "Couldnt open cfg" , NULL, NULL);
		exit(0);
	}

	string err;
	if (!CFGHelper::IsCFGComplete(err))
	{
		string msg = "CFG file incomplete: " + err;
		MessageBox(NULL, msg.c_str(), NULL, NULL);
		exit(0);
	}

	if (!dbCtrlr.openDB(CFGHelper::dbPath))
	{
		string msg = "couldnt open db form file at: " + CFGHelper::dbPath;
		MessageBox(NULL, msg.c_str(), NULL, NULL);
		exit(0);
	}

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
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
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DBLOOKUP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_DBLOOKUP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
	mainInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   

   if (!hWnd)
   {
      return FALSE;
   }

   // createSelectOrDeleteBox(hWnd);

   InitInputFields(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   RECT rcClient;                       // The parent window's client area.
   GetClientRect(hWnd, &rcClient);

   vector<string> colNames;
   colNames.push_back("path");
   listView= CreateListView(hWnd, colNames,rcClient.right /2 , rcClient.top+20, rcClient.right/2, rcClient.bottom+20,500);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

	case WM_NOTIFY:
		//return(NotifyHandler(hWnd, message, wParam, lParam));
		break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			string output;
			CheckInput(wParam, output);
			vector<string> data = StringUtils::Tokenize(output, "|");
			FillListViewItems(listView, data); 
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
			{
				DialogBox(mainInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				/*//////test!!!
				ListView_DeleteAllItems(listView);
				vector<string> testVec;
				testVec.push_back("message 1,hety");
				testVec.push_back("super street fighter II turbo");
				testVec.push_back("gimme all da pornz");
				testVec.push_back("howag");
				testVec.push_back("wtf facebook?!");


				FillListViewItems(listView, testVec);*/
			}
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
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

HWND CreateListView(HWND hwndParent, vector<string> colNames,int x, int y, int w, int h, int size)
{
	INITCOMMONCONTROLSEX icex;           // Structure for control initialization.
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);
	LV_COLUMN lvC;      // List View Column structure
	char szText[MAX_PATH];

	// Create the list-view window in report view with label editing enabled.
	HWND hWndListView = CreateWindow(WC_LISTVIEW,
		"resultsList",
		WS_VISIBLE | WS_CHILD | LVS_REPORT | LVS_EDITLABELS | WS_BORDER | LVS_SHOWSELALWAYS,
		x, y,w,h,
		hwndParent,
		(HMENU)IDM_LIST_VIEW_RESULTS,
		mainInst,
		NULL);
	// Add the columns.
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.pszText = szText;
	for (int index = 0; index < NUM_COL; index++)
	{
		lvC.cx = size;            // width of the column, in pixels
		lvC.iSubItem = index;
		lvC.pszText = (LPSTR)colNames[index].c_str();
		ListView_InsertColumn(hWndListView, index, &lvC);
	}
	return (hWndListView);
}

BOOL FillListViewItems(HWND hWndListView, vector<string> items)
{
	LVITEM lvI;

	// Initialize LVITEM members that are common to all items.
	lvI.mask = LVIF_TEXT | LVIF_PARAM; //// Initialize LV_ITEM members that are common to all items.
	lvI.stateMask = 0;
	lvI.state = 0;

	// Initialize LVITEM members that are different for each item.
	for (size_t index = 0; index < items.size(); index++)
	{
		lvI.pszText = (LPSTR)items[index].c_str();
		lvI.iSubItem = 0;
		lvI.iItem = (int)index;
		lvI.cchTextMax = items[index].size();
		//lvI.iImage = index;
		//lvI.pszText = LPSTR_TEXTCALLBACK; // Sends an LVN_GETDISPINFO message.
		//lvI.lParam = (LPARAM)&doubleArray[index];	// item data
		
		// Insert items into the list.
		if (ListView_InsertItem(hWndListView, &lvI) == -1)
			return FALSE;

		
		for (int iSubItem = 1; iSubItem < NUM_COL; iSubItem++)
		{
			ListView_SetItemText(hWndListView,index,iSubItem,LPSTR_TEXTCALLBACK);
		}
	}

	return TRUE;
}
/*
LRESULT NotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDM_LIST_VIEW_RESULTS)
		return 0L;

	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	//NM_LISTVIEW *pNm = (NM_LISTVIEW *)lParam;
	HOUSEINFO *pHouse = (HOUSEINFO *)(pLvdi->item.lParam);

	switch (pLvdi->hdr.code)
	{
	case LVN_GETDISPINFO:

		switch (pLvdi->item.iSubItem)
		{
		case 0:     // Address
			pLvdi->item.pszText = pHouse->szAddress;
			break;

		case 1:     // City
			pLvdi->item.pszText = pHouse->szCity;
			break;

		default:
			break;
		}
		break;

	case LVN_BEGINLABELEDIT:
	{
		HWND hWndEdit;

		// Get the handle to the edit box.
		hWndEdit = (HWND)SendMessage(hWnd, LVM_GETEDITCONTROL,
			0, 0);
		// Limit the amount of text that can be entered.
		SendMessage(hWndEdit, EM_SETLIMITTEXT, (WPARAM)20, 0);
	}
	break;

	case LVN_ENDLABELEDIT:
		// Save the new label information
		if ((pLvdi->item.iItem != -1) &&
			(pLvdi->item.pszText != NULL))
			lstrcpy(pHouse->szAddress, pLvdi->item.pszText);
		break;

	case LVN_COLUMNCLICK:
		// The user clicked on one of the column headings - sort by
		// this column.
		//ListView_SortItems(pNm->hdr.hwndFrom,
		//	ListViewCompareProc,
		//	(LPARAM)(pNm->iSubItem));
		break;

	default:
		break;
	}
	return 0L;
}
*/