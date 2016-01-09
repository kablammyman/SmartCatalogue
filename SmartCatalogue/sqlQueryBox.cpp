#include "stdafx.h"
#include "MainApp.h"
#include <string>
#include "SmartCatalogue.h"

// Message handler for doing a gerneic SQL query...window dims are in resource.rc
INT_PTR CALLBACK SQLQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		//CreateWindowA("BUTTON", "test",  WS_VISIBLE | WS_CHILD,MainApp::Instance()->baseX, MainApp::Instance()->baseY,   MainApp::Instance()->buttonW,MainApp::Instance()->buttonH,hDlg,(HMENU)(FILE_OPEN_1),NULL,0);
		if (MainApp::Instance()->curDBCommand.empty())
			MainApp::Instance()->curDBCommand = "SELECT path FROM galleries where modelFirstName = \"\"";
		CreateWindow(TEXT("Edit"), TEXT(MainApp::Instance()->curDBCommand.c_str()), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER, 0, 0, MAX_PATH * 10, 20, hDlg, (HMENU)FILE_PATH_1, NULL, NULL);

		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			//use file browser
			//masterFilePath = BrowseToFile(hWnd,"C:\\");
			//set the string to what the file broser gave us so we can see it on screen
			//SetDlgItemText(hWnd, FILE_PATH_1, TEXT(masterFilePath.c_str()));

			char buffer[MAX_PATH];
			//get the data in the input filed for master file
			GetDlgItemText(hDlg, FILE_PATH_1, buffer, MAX_PATH);
			//fill string with data in text box (incase they typed in a path)
			MainApp::Instance()->curDBCommand = (char *)buffer;
			string output;
			MainApp::Instance()->dbCtrlr.executeSQL(MainApp::Instance()->curDBCommand, output);

			EndDialog(hDlg, LOWORD(wParam));
			//UpdateWindow();
			RedrawWindow(MainApp::Instance()->getMainWindowHandle(), NULL, NULL, RDW_INVALIDATE);
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}