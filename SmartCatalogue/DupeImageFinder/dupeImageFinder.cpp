// dupeImageFinder.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <map>
#include <thread>
#include "Shellapi.h" //shellExecute
#include <fstream>

#include "dupeImageFinder.h"
#include "ProgressBar.h"
#include "similarImage.h"
#include "myFileDirDll.h"
#include "Utils.h"


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	Utils::filePathBase = Utils::getExePath();
	Utils::loadCFGFile();

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_DUPEIMAGEFINDER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	InitCommonControls();

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DUPEIMAGEFINDER));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
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

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DUPEIMAGEFINDER));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DUPEIMAGEFINDER);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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
	hInst = hInstance; // Store instance handle in our global variable

	mainWindowHandle = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!mainWindowHandle)
	{
		return FALSE;
	}

	ShowWindow(mainWindowHandle, nCmdShow);
	UpdateWindow(mainWindowHandle);
	RECT rc;
	GetClientRect(mainWindowHandle, &rc);
	//hTree = CreateWindowEx(0, WC_TREEVIEW, _T(""), WS_VISIBLE | WS_CHILD | WS_BORDER | WS_HSCROLL | WS_VSCROLL | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS, 0, 45, 216, 532, mainWindowHandle, NULL, hInst, NULL);

	hwndTree = CreateWindowEx(WS_EX_CLIENTEDGE,WC_TREEVIEW,0,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
		0, 30, rc.right/2, rc.bottom,
		mainWindowHandle, NULL, hInstance, NULL);
	
	InitTreeViewItems(hwndTree);

	thread t1(findAllTheDupes, hwndTree);
	t1.detach();

	//or i can put all the threads in a vector to join later
	/*
	std::vector<std::thread> allThreads;
	for (int i = 0; i < 10; ++i) {
	allThreads.emplace_back(myfunction, i, param2, param3);
	}

	// Do something else...

	for (auto& t : allThreads) {
	t1.join();
	}
	*/
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

#define UM_CHECKSTATECHANGE (WM_USER + 100)

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	
	//this is a fancy way to say "right click"
	case WM_CONTEXTMENU:
		lilMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
		break;

	/*case WM_NOTIFY:
	{
		case IDC_TREE1:
		
			if (((LPNMHDR)lParam)->code == NM_RCLICK) // Right Click
				lilMenu((HWND)wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	}*/
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

HTREEITEM AddItemToTree(HWND hwndTV, wchar_t* lpszItem, int nLevel)
{
	TVITEM tvi;
	TVINSERTSTRUCT tvins;
	static HTREEITEM hPrev = (HTREEITEM)TVI_FIRST;
	static HTREEITEM hPrevRootItem = NULL;
	static HTREEITEM hPrevLev2Item = NULL;
	HTREEITEM hti;

	tvi.mask = TVIF_TEXT | TVIF_IMAGE| TVIF_SELECTEDIMAGE | TVIF_PARAM;

	// Set the text of the item. 
	tvi.pszText = lpszItem;
	tvi.cchTextMax = sizeof(tvi.pszText) / sizeof(tvi.pszText[0]);

	// Assume the item is not a parent item, so give it a 
	// document image. 
	//tvi.iImage = g_nDocument;
	//tvi.iSelectedImage = g_nDocument;

	// Save the heading level in the item's application-defined 
	// data area. 
	tvi.lParam = (LPARAM)nLevel;
	tvins.item = tvi;
	tvins.hInsertAfter = hPrev;

	// Set the parent item based on the specified level. 
	if (nLevel == 1)
		tvins.hParent = TVI_ROOT;
	else if (nLevel == 2)
		tvins.hParent = hPrevRootItem;
	else
		tvins.hParent = hPrevLev2Item;

	// Add the item to the tree-view control. 
	hPrev = (HTREEITEM)SendMessage(hwndTV, TVM_INSERTITEM,
		0, (LPARAM)(LPTVINSERTSTRUCT)&tvins);

	if (hPrev == NULL)
		return NULL;

	// Save the handle to the item. 
	if (nLevel == 1)
		hPrevRootItem = hPrev;
	else if (nLevel == 2)
		hPrevLev2Item = hPrev;

	// The new item is a child item. Give the parent item a 
	// closed folder bitmap to indicate it now has child items. 
	if (nLevel > 1)
	{
		hti = TreeView_GetParent(hwndTV, hPrev);
		tvi.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE;
		tvi.hItem = hti;
		//tvi.iImage = g_nClosed;
		//tvi.iSelectedImage = g_nClosed;
		TreeView_SetItem(hwndTV, &tvi);
	}

	return hPrev;
}

// Extracts heading text and heading levels from a global 
// array and passes them to a function that adds them as
// parent and child items to a tree-view control. 
// Returns TRUE if successful, or FALSE otherwise. 
// hwndTV - handle to the tree-view control. 

BOOL InitTreeViewItems(HWND hwndTV)
{	
	dbCtrlr.openDatabase(Utils::dbPath);
	
	wstring rootNodeOutput(Utils::dbPath.begin(), Utils::dbPath.end());
	AddItemToTree(hwndTV, (LPTSTR)rootNodeOutput.c_str(), 1);

	return TRUE;
}

//get the main window handle, x and y cords of mouse
void lilMenu(HWND handle, int x, int y)
{
	if (handle != hwndTree)
		return;

	printf("%d,%d",x, y);

	POINT point;
	point.x = (SHORT)x;
	point.y = (SHORT)y;
	::ScreenToClient(handle, &point);

	// find out the item under the cursor
	TVHITTESTINFO hitTestInfo = { 0 };
	hitTestInfo.pt.x = point.x;
	hitTestInfo.pt.y = point.y;
	hitTestInfo.flags = TVHT_ONITEM;

	//HTREEITEM hItem = TreeView_GetSelection(handle);

	TreeView_HitTest(handle, &hitTestInfo);
	HTREEITEM hItem = hitTestInfo.hItem;
	
	wchar_t buffer[MAX_PATH];
	TVITEM tvItem;
	tvItem.hItem = hItem;
	tvItem.cchTextMax = MAX_PATH;
	tvItem.pszText = buffer;
	tvItem.mask = TVIF_TEXT | TVIF_HANDLE;

	wstring selectedText;
	if (TreeView_GetItem(handle, &tvItem))
	{
		selectedText = tvItem.pszText;
	}


	SendDlgItemMessage(handle, IDC_TREE1, TVM_SELECTITEM, TVGN_CARET, (LPARAM)hItem);
	SendDlgItemMessage(handle, IDC_TREE1, TVM_SELECTITEM, TVGN_DROPHILITE, (LPARAM)hItem);
	TreeView_EnsureVisible(handle, hItem);

	HMENU menu = ::LoadMenu(::GetModuleHandle(0), MAKEINTRESOURCE(IDR_MENU_TREEVIEW));
	HMENU subMenu = ::GetSubMenu(menu, 0);
	int command = ::TrackPopupMenu(subMenu, TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, x, y, 0, mainWindowHandle, 0);

	if (command == ID_TREEVIEW_VIEW_ITEM)
	{
		//ShellExecute(0, 0, L"\\\\OPTIPLEX-745\\photos\\porno pics\\mega sites\\twistys\\models\\Valentina Nappi\\black flower robe blue flower undies\\8_628.jpg", 0, 0, SW_SHOW);
		if(selectedText.size() > 3)// I guess the min length is 4... C:\a can be a legit file
			ShellExecute(0, 0, selectedText.c_str(), 0, 0, SW_SHOW);
	}
	else if (command == ID_TREEVIEW_DELETE_ITEM)
	{
		// delete if it is not root
		//if (item != treeView.getRoot())
		//	treeView.deleteItem(item);
		if (selectedText.size() > 3)// I guess the min length is 4... C:\a can be a legit file
		{
			string fileName(selectedText.begin(), selectedText.end());
			MyFileDirDll::deleteFile(fileName);
		}
		
	}
	else if (command == ID_TREEVIEW_OPEN_DIR)
	{
		if (selectedText.size() < 3)
			return;

		size_t found = selectedText.find_last_of(L"/\\");
		//std::cout << " path: " << str.substr(0, found) << '\n';
		//std::cout << " file: " << str.substr(found + 1) << '\n';
		wstring path = selectedText.substr(0, found);
		ShellExecute(NULL, L"explore", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
	}

	
}

void addNewItemToTree(HWND hwndTV, string curMD5,  string quereyOutput)
{
	HTREEITEM hti;
	//put all dupes for this image in a list
	vector<DatabaseController::dbDataPair> dupeList;
	dbCtrlr.removeTableNameFromOutput(quereyOutput, 2, 1, 2, dupeList);

	//get the path and filename for current image
	string querey = "SELECT  Images.fileName, Gallery.path FROM Images INNER JOIN Gallery ON Gallery.ID = Images.galleryID WHERE Images.MD5 = '";
	querey += curMD5;
	querey += "';";
	string output;
	dbCtrlr.executeSQL(querey, output);

	//make this "branch" from the current image file name and path
	vector<DatabaseController::dbDataPair> branchName;
	dbCtrlr.removeTableNameFromOutput(output, 2, 1, 2, branchName);
	string imagePath = (branchName[0].second + branchName[0].first);
	wstring temp(imagePath.begin(), imagePath.end());
	hti = AddItemToTree(hwndTV, (LPTSTR)temp.c_str(), 2);

	//all the children of this branch will be the list of dupes
	for (size_t j = 0; j < dupeList.size(); j++)
	{
		string fullPath = (dupeList[j].second + dupeList[j].first);
		wstring subTemp(fullPath.begin(), fullPath.end());
		hti = AddItemToTree(hwndTV, (LPTSTR)subTemp.c_str(), 3);
	}
}

void findAllTheDupes(HWND hwndTV)
{
	int start_s = clock();
	string output;
	//get a list of all images
	string querey = "SELECT hash, MD5 FROM Images;";
	dbCtrlr.executeSQL(querey, output);


	vector<DatabaseController::dbDataPair> hashes;
	dbCtrlr.removeTableNameFromOutput(output, 2, 1, 2, hashes);

	ProgressBar progress(hwndTV, 10, 1, 200, hashes.size());
	//now compare each image in the list to the db, making sure not to include it self

	for (size_t i = 0; i < hashes.size(); i++)
	{
		progress.updateProgressBar(i);
		querey = "SELECT  Images.fileName, Gallery.path FROM Images INNER JOIN Gallery ON Gallery.ID = Images.galleryID WHERE Images.MD5 != '";
		querey += hashes[i].second;
		querey += "' AND hammingDistance('";
		querey += hashes[i].first;
		querey += "',hash) < ";
		querey += to_string(/*simImage.getMinHammingDist() + 2*/ 3);
		querey += ";";
		dbCtrlr.executeSQL(querey, output);
		//we have a match
		if (output.find("path|") != string::npos)
		{
			addNewItemToTree(hwndTV, hashes[i].second, output);
		}
	}


	//if (hti == NULL)
	//	return FALSE;
	int stop_s = clock();
	double milis = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	string timeTaken = Utils::getTimeStamp(milis);

	ofstream myfile;
	myfile.open("dupeImageFinder.txt");
	myfile << "parsed " << Utils::dbPath << endl;
	myfile << timeTaken << endl;
	myfile.close();
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


/*
BOOL InitTreeViewItems(HWND hwndTV)
{
HTREEITEM hti;

map<int, vector<int>> dupeList;
SimilarImage simImg;
DatabaseController dbCtrlr;

dbCtrlr.openDatabase(Utils::dbPath);
string output;
string command = "SELECT hash, MD5 FROM Images;";
dbCtrlr.executeSQL(command, output);

vector<DatabaseController::dbDataPair> hashes;
dbCtrlr.removeTableNameFromOutput(output,2,1,2,hashes);

//vector<vector<string>> allImages;
//dbCtrlr.parseDBOutput(output, 2, allImages);

simImg.findDupes(hashes, dupeList);

wstring rootNodeOutput(Utils::dbPath.begin(), Utils::dbPath.end());
AddItemToTree(hwndTV, (LPTSTR)rootNodeOutput.c_str(), 1);

// show content:
for (map<int, vector<int>>::iterator it = dupeList.begin(); it != dupeList.end(); ++it)
{
int index = it->first;

string output;
string md5Index = hashes[index].second;
string command = ("SELECT fileName, galleryID FROM Images WHERE MD5 = '" + md5Index+"';");
dbCtrlr.executeSQL(command, output);

vector<string> imgInfo;
dbCtrlr.removeTableNameFromOutput(output, 2, 1, imgInfo);

wstring temp(imgInfo[0].begin(), imgInfo[0].end());
hti = AddItemToTree(hwndTV, (LPTSTR)temp.c_str(), 2);

vector<int> curListOfDupes = it->second;

for (size_t i = 0; i < curListOfDupes.size(); i++)
{
//string curDir = simImg.allImages[listOfDupes[i]].path;
md5Index = hashes[curListOfDupes[i]].second;
command = ("SELECT fileName, galleryID FROM Images WHERE MD5 = '" + md5Index + "';");
dbCtrlr.executeSQL(command, output);

dbCtrlr.removeTableNameFromOutput(output, 2, 1, imgInfo);

wstring subTemp(imgInfo[0].begin(), imgInfo[0].end());
hti = AddItemToTree(hwndTV, (LPTSTR)subTemp.c_str(), 3);
}
}

//if (hti == NULL)
//return FALSE;
return TRUE;
}
*/