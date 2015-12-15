#include "stdafx.h"
#include "MainApp.h"
#include <string>
#include "SmartCatalogue.h"

//helper for the AddRecordBox function
void fillDirWithPath(HWND hDlg)
{
	char buffer[MAX_PATH];

	//fill string with data in text box (incase they typed in a path)
	GetDlgItemText(hDlg, FILE_PATH_1, buffer, MAX_PATH);
	MainApp::Instance()->dir = (char *)buffer;


	SetDlgItemText(hDlg, FILE_PATH_1, TEXT(MainApp::Instance()->dir.c_str()));

	
}
// Message handler for add record...window dims are in resource.rc
INT_PTR CALLBACK AddRecordBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	std::string newPath;

	UNREFERENCED_PARAMETER(lParam);


	switch (message)
	{
	case WM_INITDIALOG:
		//MOVED ALL TO RECORUCE FILE, check IDD_ADDBOX	
		SetDlgItemText(hDlg, FILE_PATH_1, TEXT(MainApp::Instance()->dir.c_str()));
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case BROWSE:
			//use folder browser
			newPath = MainApp::Instance()->DoBrowseFolder(hDlg);
			if (!newPath.empty())
				MainApp::Instance()->dir = newPath;//only set this if we choose a legit dir
								  //set the string to what the file broser gave us so we can see it on screen
			SetDlgItemText(hDlg, FILE_PATH_1, TEXT(MainApp::Instance()->dir.c_str()));
			break;
		case FIND_EMPTY_DIRS:
			fillDirWithPath(hDlg);
			//MainApp::Instance()->GetAllEmptyDirs();
			EndDialog(hDlg, LOWORD(wParam));
			RedrawWindow(MainApp::Instance()->getMainWindowHandle(), NULL, NULL, RDW_INVALIDATE);
			break;
		case FIND_WRONG_PATH:
			fillDirWithPath(hDlg);
			//MainApp::Instance()->GetAllBadPaths();
			EndDialog(hDlg, LOWORD(wParam));
			RedrawWindow(MainApp::Instance()->getMainWindowHandle(), NULL, NULL, RDW_INVALIDATE);
			break;
		case FIND_IMAGE_PATH:
			fillDirWithPath(hDlg);
			//MainApp::Instance()->GetAllImageDirs();
			EndDialog(hDlg, LOWORD(wParam));
			RedrawWindow(MainApp::Instance()->getMainWindowHandle(), NULL, NULL, RDW_INVALIDATE);
			break;
		case FIND_NEW_PATH:
			fillDirWithPath(hDlg);
			//MainApp::Instance()->GetAllNewDirs();
			EndDialog(hDlg, LOWORD(wParam));
			RedrawWindow(MainApp::Instance()->getMainWindowHandle(), NULL, NULL, RDW_INVALIDATE);
			break;
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}