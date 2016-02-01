// dupeImageFinder.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <map>
#include <thread>
#include "Shellapi.h" //shellExecute
#include <fstream>

#include "dupeImageFinder.h"

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

	hwndTree = CreateWindowEx(WS_EX_CLIENTEDGE,WC_TREEVIEW,0,
		WS_CHILD | WS_VISIBLE | WS_BORDER | WS_HSCROLL | WS_VSCROLL | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS,
		0, 30, rc.right/2, rc.bottom,
		mainWindowHandle, NULL, hInstance, NULL);
	
	InitTreeViewItems(hwndTree);
	//waitingMarquee.init(mainWindowHandle, 1, 1, 100, 100,true);
	thread t1(init);
	t1.detach();

	progress.init(mainWindowHandle, 1, 1, (rc.right / 2),100);
	statusText = CreateWindow(TEXT("Edit"), TEXT("opening DB and loading images"), WS_CHILD | WS_VISIBLE | WS_BORDER, (rc.right / 2), 0, 1000, 30, mainWindowHandle, (HMENU)IDC_STATUS_TEXT, NULL, NULL);

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

	hwndTreeView = hwndTV;

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
		deleteImage(selectedText);
		TreeView_DeleteItem(handle, hItem);
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
	else if (command == ID_REMOVE_FROM_TREE)
	{
		TreeView_DeleteItem(handle, hItem);
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

	//not sure how this happens, but it does wqhen mulit thraded...
	//oh...i see...this is why...lol
	/*
	Make sure you're compiling SQLite with -DTHREADSAFE=1.
	Make sure that each thread opens the database file and keeps its own sqlite structure.
	Make sure you handle the likely possibility that one or more threads collide when they access the db file at the same time: handle SQLITE_BUSY appropriately.
	Make sure you enclose within transactions the commands that modify the database file, like INSERT, UPDATE, DELETE, and others.
	*/
	if (output.empty())
		return;

	//make this "branch" from the current image file name and path
	vector<DatabaseController::dbDataPair> branchName;
	dbCtrlr.removeTableNameFromOutput(output, 2, 1, 2, branchName);
	string imagePath = (branchName[0].second + branchName[0].first);
	
	string text = ("lastMatch: " + imagePath);
	SetWindowTextA(statusText, text.c_str());

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

void init()
{
	start_s = clock();
	string output;
	//get a list of all images
	string querey = "SELECT hash, MD5 FROM Images;";
	dbCtrlr.executeSQL(querey, output);
	dbCtrlr.removeTableNameFromOutput(output, 2, 1, 2, hashes);
	progress.setRange(hashes.size());
	thread dupeSearch(mainLogic);
	dupeSearch.detach();
}

void deleteImage(wstring image)
{
	// delete if it is not root
	//if (item != treeView.getRoot())
	//	treeView.deleteItem(item);
	if (image.size() < 3)// I guess the min length is 4... C:\a can be a legit file
		return;

	string fileName(image.begin(), image.end());
	/*if (!MyFileDirDll::deleteFile(fileName))
	{
		wstring err = (L"Error deleting " + image);
		MessageBox(NULL, err.c_str() , NULL, NULL);
		return;
	}*/
	string output, querey;
	querey = "DELETE FROM Images WHERE  Images.fileName = '";
	querey += MyFileDirDll::getFileNameFromPathString(fileName);  
	querey += "' AND galleryID IS (SELECT galleryID FROM Gallery WHERE path = '";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(fileName);
	querey += "\\');"; //the DB has the back slashes in the path, so i need to include them here
	
	dbCtrlr.executeSQL(querey, output);
	cout << output << endl;
}

void deleteGallery(wstring path)
{
	string output, querey;
	string fileName(path.begin(), path.end());
	querey = "DELETE FROM Images  WHERE galleryID is (select id from Gallery where path = '";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(fileName);
	querey += "\\');";

	dbCtrlr.executeSQL(querey, output);

	querey = "delete FROM Gallery where path = '";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(fileName);
	querey += "\\';";

	dbCtrlr.executeSQL(querey, output);
}
void mainLogic()
{
	SetWindowTextA(statusText, "searching for dupe images...");
	//i put this here so the app will not hourglass as it does its main calcls
	//now compare each image in the list to the db, making sure not to include it self
	string output, querey;
	for (size_t i = 0; i < hashes.size(); i++)
	{
		querey = "SELECT  Images.fileName, Gallery.path FROM Images INNER JOIN Gallery ON Gallery.ID = Images.galleryID WHERE Images.MD5 != '";
		querey += hashes[i].second;
		querey += "' AND hammingDistance('";
		querey += hashes[i].first;
		querey += "',hash) < ";
		querey += to_string(/*simImage.getMinHammingDist() + 2*/ 7);
		querey += ";";
		dbCtrlr.executeSQL(querey, output);
		//we have a match
		if (output.find("path|") != string::npos)
		{
			//thread newMatch(addNewItemToTree, hwndTreeView, hashes[i].second, output);
			//newMatch.detach();
			addNewItemToTree(hwndTreeView, hashes[i].second, output);
		}
		progress.updateProgressBar(i);
	}

	finish();
}

void finish()
{
	stop_s = clock();
	double milis = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	string timeTaken = Utils::getTimeStamp(milis);

	ofstream myfile;
	myfile.open("dupeImageFinder.txt");
	if (!myfile.is_open())
		return;
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