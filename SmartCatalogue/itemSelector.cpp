#include "stdafx.h"
#include "MainApp.h"
#include "SmartCatalogue.h"
#include <Shellapi.h>

void openSelectedPath(string path)
{
	if (path[path.size() - 1] == '\r')//the is the carrage return
		path.pop_back();

	if(path.find_last_of("path|") != string::npos)
		path.erase(0, 5);   

	/*HINSTANCE x =*/ ShellExecute(NULL, "explore", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
	//printf("%d", x);
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
		// This is where we set up the dialog box, and initialise any default values

	//	SetDlgItemText(hwnd, IDC_TEXT, "This is a string");
	//	SetDlgItemInt(hwnd, IDC_NUMBER, 5, FALSE);
	{
		size_t nTimes = MainApp::Instance()->curDBWindowData.size();
		// Then we get the string they entered
		// First we need to find out how long it is so that we can
		// allocate some memory

	//	int len = GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT));
	//	if (len > 0)

			// Now we allocate, and get the string into our buffer



		// Now we add the string to the list box however many times
		// the user asked us to.

		for (size_t i = 0; i < nTimes; i++)
		{
			string curLine = MainApp::Instance()->curDBWindowData[i];
			LRESULT index = SendDlgItemMessage(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)curLine.c_str());

			// Here we are associating the value nTimes with the item 
			// just for the heck of it, we'll use it to display later.
			// Normally you would put some more useful data here, such
			// as a pointer.
			SendDlgItemMessage(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)index, (LPARAM)nTimes);
		}

	}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_ADD:
		{
			// When somebody clicks the Add button, first we get the number of
			// they entered


		}
		break;
		case IDC_REMOVE:
		{
			// When the user clicks the Remove button, we first get the number
			// of selected items

			HWND hList = GetDlgItem(hwnd, IDC_LIST);
			LRESULT count = SendMessage(hList, LB_GETSELCOUNT, 0, 0);
			if (count != LB_ERR)
			{
				if (count != 0)
				{
					// And then allocate room to store the list of selected items.
					int *buf = (int*)GlobalAlloc(GPTR, sizeof(int) * count);
					SendMessage(hList, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);

					// Now we loop through the list and remove each item that was
					// selected.  

					// WARNING!!!  
					// We loop backwards, because if we removed items
					// from top to bottom, it would change the indexes of the other
					// items!!!

					for (LRESULT i = count - 1; i >= 0; i--)
					{
						SendMessage(hList, LB_DELETESTRING, (WPARAM)buf[i], 0);
					}

					GlobalFree(buf);
				}
				else
				{
					MessageBox(hwnd, "No items selected.", "Warning", MB_OK);
				}
			}
			else
			{
				MessageBox(hwnd, "Error counting items :(", "Warning", MB_OK);
			}
		}
		break;
		case IDC_CLEAR:
			SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
			break;
		case IDC_LIST:
			switch (HIWORD(wParam))
			{
			case LBN_SELCHANGE:
			{
				// Get the number of items selected.

				HWND hList = GetDlgItem(hwnd, IDC_LIST);
				LRESULT numItemsSelected = SendMessage(hList, LB_GETSELCOUNT, 0, 0);
				if (numItemsSelected != LB_ERR)
				{
					// We only want to continue if one and only one item is
					// selected.

					if (numItemsSelected == 1)
					{
						// Since we know ahead of time we're only getting one
						// index, there's no need to allocate an array.

						int index;
						
						LRESULT err = SendMessage(hList, LB_GETSELITEMS, (WPARAM)1, (LPARAM)&index);
						if (err != LB_ERR)
						{
							LRESULT count = SendMessage(hList, LB_GETCOUNT, 0, 0);

							// Get the data we associated with the item above
							//int data = SendMessage(hList, LB_GETITEMDATA, (WPARAM)index, 0);
							char buffer[1024] = { 0 };

							//////////////////////////////////////////////////
							int iSelected = -1;
							// go through the items and find the first selected one
							for (int i = 0; i < count; i++)
							{
								// check if this item is selected or not..
								if (SendMessage(hList, LB_GETSEL, i, 0) > 0)
								{
									// yes, we only want the first selected so break.
									iSelected = i;
									break;
								}
							}

							// get the text of the selected item
							if (iSelected != -1)
								SendMessage(hList, LB_GETTEXT, (WPARAM)iSelected, (LPARAM)buffer);
							//////////////////////////////////////////////////
							string path = buffer;
							openSelectedPath(path);
						//	SetDlgItemInt(hwnd, IDC_SHOWCOUNT, data, FALSE);
						}
						else
						{
							MessageBox(hwnd, "Error getting selected item :(", "Warning", MB_OK);
						}
					}
					else
					{
						// No items selected, or more than one
						// Either way, we aren't going to process this.
						SetDlgItemText(hwnd, IDC_SHOWCOUNT, "-");
					}
				}
				else
				{
					MessageBox(hwnd, "Error counting items :(", "Warning", MB_OK);
				}
			}
			break;
			}
			break;
		}
		break;
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		break;
	case WM_SIZE:
	{
		HWND hTool;
		RECT rcTool;
		int iToolHeight;

//		HWND hStatus;
//		RECT rcStatus;
//		int iStatusHeight; //never used

		HWND hMDI;
		int iMDIHeight;
		RECT rcClient;

		// Size toolbar and get height

		hTool = GetDlgItem(hwnd, IDD_MAIN_SELECTOR);
		SendMessage(hTool, TB_AUTOSIZE, 0, 0);

		GetWindowRect(hTool, &rcTool);
		iToolHeight = rcTool.bottom - rcTool.top;


		// Calculate remaining height and size edit
		GetClientRect(hwnd, &rcClient);

		iMDIHeight = rcClient.bottom - iToolHeight /*- iStatusHeight*/;

		hMDI = GetDlgItem(hwnd, IDC_MAIN_MDI);
		SetWindowPos(hMDI, NULL, 0, iToolHeight, rcClient.right, iMDIHeight, SWP_NOZORDER);
	}
	default:
		return (INT_PTR)FALSE;
	}
	return (INT_PTR)TRUE;
}
