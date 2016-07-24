#include "stdafx.h"
//#include "Resource.h"
#include "DBLookup.h"
#include <string>
#include <Windows.h>

using namespace std;
HWND addOrDeleteBoxHandle = NULL;
HWND SQLQureyTextHandle;
HWND RadioButtonGroup;
HWND CheckBoxButtonGroup;

HWND selectRadioButton;
HWND deleteRadioButton;
HWND categoryCheck;
HWND categoryText;

HWND websiteCheck;
HWND websiteText;

HWND subwebsiteCheck;
HWND subwebsiteText;

HWND modelCheck;
HWND modelText;

HWND galleryCheck;
HWND galleryText;

HWND metaCheck;
HWND metaText;

std::string intToSQLCatMap[NUM_CHECKS];



void createSelectOrDeleteBox(HWND hWnd)
{
	addOrDeleteBoxHandle = CreateDialog(mainInst, MAKEINTRESOURCE(IDD_QUICK_QUERY_BOX), hWnd, selectOrDeleteQuereyBox);

	if (addOrDeleteBoxHandle == NULL)
	{
		int x = GetLastError();
		printf("error: %d", x);
	}
	ShowWindow(addOrDeleteBoxHandle, SW_SHOW);
}

void InitInputFields(HWND hDlg)
{
	intToSQLCatMap[0] = CATERGORY_STRING;
	intToSQLCatMap[1] = WEBSITE_STRING;
	intToSQLCatMap[2] = SUB_WEBSITE_STRING;
	intToSQLCatMap[3] = MODEL_FIRST_NAME_STRING;
	intToSQLCatMap[4] = MODEL_LAST_NAME_STRING;
	intToSQLCatMap[5] = GALLERY_STRING;
	intToSQLCatMap[6] = META_STRING;

	int baseX = 5, baseY = 100, stepY = 32;
	int editSize = stepY - 2;
	int buttonWidth = (baseX * 2) + 100;

	//CreateWindow(TEXT("Edit"), TEXT(curDBCommand.c_str()), WS_CHILD | WS_VISIBLE | WS_BORDER,0, 0, MAX_PATH*10, 20, hDlg, (HMENU)FILE_PATH_1, NULL, NULL);

	/*RadioButtonGroup = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("Button"), TEXT("select or delete?"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 2, 2, 290, 50, hDlg, (HMENU)11, NULL, NULL);
	selectRadioButton = CreateWindow(TEXT("BUTTON"), TEXT("Select"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, baseX, 20, 100, 25, RadioButtonGroup, (HMENU)IDC_SELECT_RADIO, NULL, NULL);
	deleteRadioButton = CreateWindow(TEXT("BUTTON"), TEXT("Delete"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON, baseX * 30, 20, 100, 25, RadioButtonGroup, (HMENU)IDC_DELETE_RADIO, NULL, NULL);
	SendMessage(selectRadioButton, BM_SETCHECK, BST_CHECKED, 0);*/
	SQLQureyTextHandle = hDlg;
	CreateWindow(TEXT("Edit"), TEXT("SELECT path FROM galleries where modelFirstName = \"\""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER, baseX, 30, 1000, 20, hDlg, (HMENU)IDC_SQLQUEREY_TEXT, NULL, NULL);
	CreateWindow(TEXT("BUTTON"), TEXT("Querey"), WS_VISIBLE | WS_CHILD, baseX, 60, 100, 30, hDlg, (HMENU)IDC_QUERY_BTN, NULL, NULL);

	CheckBoxButtonGroup = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("Button"), TEXT("Search Criteria"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 2, baseY, 290, 450, hDlg, (HMENU)11, NULL, NULL);
	categoryCheck = CreateWindow(TEXT("BUTTON"), TEXT("Category"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_CATERGORY_CHECK, NULL, NULL);
	categoryText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY), 1000, editSize, CheckBoxButtonGroup, (HMENU)IDC_CATERGORY_TEXT, NULL, NULL);

	websiteCheck = CreateWindow(TEXT("BUTTON"), TEXT("Website"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 2), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_WEBSITE_CHECK, NULL, NULL);
	websiteText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 2), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_WEBSITE_TEXT, NULL, NULL);

	subwebsiteCheck = CreateWindow(TEXT("BUTTON"), TEXT("sub Website"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 3), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_SUB_WEBSITE_CHECK, NULL, NULL);
	subwebsiteText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 3), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_SUB_WEBSITE_TEXT, NULL, NULL);

	modelCheck = CreateWindow(TEXT("BUTTON"), TEXT("First Name"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 4), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_MODEL_FIRST_NAME_CHECK, NULL, NULL);
	modelText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 4), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_MODEL_FIRST_NAME_TEXT, NULL, NULL);

	modelCheck = CreateWindow(TEXT("BUTTON"), TEXT("Last Name"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 5), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_MODEL_LAST_NAME_CHECK, NULL, NULL);
	modelText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 5), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_MODEL_LAST_NAME_TEXT, NULL, NULL);

	galleryCheck = CreateWindow(TEXT("BUTTON"), TEXT("gallery"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 6), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_GALLERY_CHECK, NULL, NULL);
	galleryText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 6), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_GALLERY_TEXT, NULL, NULL);

	metaCheck = CreateWindow(TEXT("BUTTON"), TEXT("Meta data"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 7), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_META_CHECK, NULL, NULL);
	metaText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 7), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_META_TEXT, NULL, NULL);

	CreateWindow(TEXT("BUTTON"), TEXT("execute"), WS_VISIBLE | WS_CHILD, baseX, baseY + (stepY * 11), 100, 50, hDlg, (HMENU)IDC_EXECUTE_QUICK_QUERY_BTN, NULL, NULL);

}
BOOL CheckInput(WPARAM wParam)
{
	if (wParam == IDC_QUERY_BTN)
	{
		char buffer[MAX_PATH];

		int x = GetDlgItemText(SQLQureyTextHandle, IDC_SQLQUEREY_TEXT, buffer, MAX_PATH);
		if (x == 0)
		{
			x = GetLastError();
			printf("error: %d", x);
		}
		string output;
		dbCtrlr.executeSQL(buffer, output);
		return (INT_PTR)TRUE;
	}
	else if(wParam == IDC_EXECUTE_QUICK_QUERY_BTN)
	{
		std::string SQLQuery = "";
	//	if (IsDlgButtonChecked(RadioButtonGroup, IDC_SELECT_RADIO))
			SQLQuery += "SELECT path FROM galleries WHERE ";
	//	else if (IsDlgButtonChecked(RadioButtonGroup, IDC_DELETE_RADIO))
	//		SQLQuery += "DELETE FROM galleries WHERE ";

		int numCatogories = 0;
		for (int i = 0; i < NUM_CHECKS; i++)
		{

			//if (IsDlgButtonChecked(CheckBoxButtonGroup, IDC_CATERGORY_CHECK + i))
			{
				char buffer[512] = { 0 };
				GetDlgItemText(CheckBoxButtonGroup, IDC_CATERGORY_TEXT + i, buffer, 512);
				if (buffer[0] == '\0')
					continue;

				if (numCatogories > 0)
					SQLQuery += " AND ";
				SQLQuery += (intToSQLCatMap[i] + "\"" + buffer + "\"");
				numCatogories++;
			}
			//MessageBox(NULL,SQLQuery.c_str(),"checks",NULL);
		}
		string output;
		dbCtrlr.executeSQL(SQLQuery, output);
		return TRUE;
	}

	return FALSE;
}
// Message handler for eitehr selecting or deleting record...window dims are in resource.rc
INT_PTR CALLBACK selectOrDeleteQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{


	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:

		InitInputFields(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
			CheckInput( wParam);

		break;
	case WM_PAINT:
		if (addOrDeleteBoxHandle != NULL)
			ShowWindow(addOrDeleteBoxHandle, SW_SHOW);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return (INT_PTR)FALSE;
}