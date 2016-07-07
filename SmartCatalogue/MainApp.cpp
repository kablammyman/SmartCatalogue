#include "stdafx.h"
#include "MainApp.h"
#include "resource.h"
#include "SmartCatalogue.h"
#include "CFGReaderDll.h"
#include <ctime> 
#include <sstream>
#include < Shlobj.h > //browse folder
#include <thread> //multi threading
#include <commdlg.h> //used for windows OPENFILENAME

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

#include <iostream>
#include <fstream>

MainApp* MainApp::m_pInstance = NULL;

MainApp* MainApp::Instance()
{
	if (!m_pInstance)   // Only allow one instance of class to be generated.
	{
		m_pInstance = new MainApp;
		m_pInstance->initVars();
	}
		return m_pInstance;
}

void MainApp::initVars()
{
	pFile = NULL;
	timeinfo = NULL;

	creationTime = NULL;     

	g_hMDIDBView = NULL;
	g_hMDIFileView = NULL;
	g_hMainWindow = NULL;
	szWindowClass = "ImageViewMain";
	szChildWindowClassDB = "ImageViewDBView";
	szChildWindowClassDir = "ImageViewFileView";

	//dir = "\\\\OPTIPLEX-745\\photos\\porno pics\\panties\\pantyflash girls";
	dir = "";
	srand((unsigned)time(0));
	long ltime = (long)time(NULL);
	int stime = (unsigned) ltime/2;
	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	
	buttonW = 100,buttonH = 40;
	baseX = 30, baseY = 20;

	winWidth = 1000;
	winHeight = 600;
	
	dbWindowX = 10;
	dbWindowY = winHeight/4;
	
	fileWindowX = (winWidth/2)+10; 
	fileWindowY = dbWindowY;

	char buffer[MAX_PATH];
    GetModuleFileName( NULL, buffer, MAX_PATH );
  
	string temp = buffer;
	size_t found = temp.find_last_of("/\\");
	string filePathBase = temp.substr(0,found);
	string cfgPath = filePathBase + "\\imageViewCfg.txt"; 
	if(!CFGUtils::readCfgFile(cfgPath,'|'))
	{
		string errorMsg = "Error opening :";
		errorMsg += cfgPath;
		MessageBox(getMainWindowHandle(), errorMsg.c_str(),"no cfg text file", MB_OKCANCEL);
	}
	mainWorkingPath = CFGUtils::getCfgStringValue("mainWorkingPath");
	dontDeleteList = CFGUtils::getCfgListValue("dontDelete");

}
void MainApp::setMainWindow(HWND hwnd)
{
	g_hMainWindow = hwnd;
}

const char *MainApp::getClassName()
{
	return szWindowClass.c_str(); 
}

HWND MainApp::getDBViewHandle()
{
	return g_hMDIDBView;
}

HWND MainApp::getFileViewHandle()
{
	return g_hMDIFileView;
}

void MainApp::setDBViewHandle(HWND hwnd)
{
	g_hMDIDBView = hwnd;
}
void MainApp::setFileViewHandle(HWND hwnd)
{
	g_hMDIFileView = hwnd;
}
void MainApp::setInstance(HINSTANCE i)
{
	hInst = i;
}

HINSTANCE MainApp::getInstance()
{
	return hInst;
}

HWND MainApp::getMainWindowHandle()
{
	return g_hMainWindow;
}

void MainApp::setMainWindowHandle(HWND hwnd)
{
	g_hMainWindow = hwnd;
}

void MainApp::calcWindowSize()
{
	RECT rect;
	if(GetWindowRect(g_hMainWindow, &rect))
	{
		windowWidth = rect.right - rect.left;
		windowHeight = rect.bottom - rect.top;
	}
}
void MainApp::DoFileOpen(HWND hwnd, std::string &name)
{
	OPENFILENAME ofn;
	//HANDLE hf;              // file handle

	TCHAR szFileName[MAX_PATH];


	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = szFileName;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFileName);
	ofn.lpstrFilter = _T("All\0*.*\0Text\0*.TXT\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;


	if(GetOpenFileName(&ofn))
	{
		name = szFileName;
	}
	
}
void MainApp::DoFileSave(HWND hwnd, std::string &name)
{
	OPENFILENAME ofn;
	TCHAR szFileName[MAX_PATH];

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = _T("Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0");
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrDefExt = _T("txt");
	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if(GetSaveFileName(&ofn))
	{
		//HWND hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
		//SaveTextFileFromEdit(hEdit, szFileName);
		 name = szFileName;
	}
}

string MainApp::DoBrowseFolder(HWND hwnd)
{
	BROWSEINFO bi = { 0 };
	/*bi.hwndOwner = GetActiveWindow();
    bi.pidlRoot = NULL; 
    bi.pszDisplayName = "what is this place";*/
	bi.pidlRoot = NULL; 
	bi.lParam = (LPARAM)dir.c_str();
    bi.ulFlags = BIF_NONEWFOLDERBUTTON| BIF_NEWDIALOGSTYLE| BIF_EDITBOX | BIF_RETURNONLYFSDIRS;
    bi.lpszTitle = _T("Pick a Directory");

    LPITEMIDLIST pidl = SHBrowseForFolder ( &bi );
	string returnString;
    if ( pidl != 0 )
    {
        // get the name of the folder
        TCHAR path[MAX_PATH];
        if ( SHGetPathFromIDList ( pidl, path ) )
        {
			returnString = path;
        }

        // free memory used
        IMalloc * imalloc = 0;
        if ( SUCCEEDED( SHGetMalloc ( &imalloc )) )
        {
            imalloc->Free ( pidl );
            imalloc->Release ( );
        }
    }
	return returnString;
}


void MainApp::callCreateNewDatabase()
{
	//this will tell the createNewDB.exe to run and also provide where it should 
	//place teh db
}


void MainApp::sendDataToEditWindow(HWND hwnd, string data)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
	SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"parsed...");
	//SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)dir.c_str());
	curDBWindowData.clear();
	string temp = "";
	string oneLine;
	for(size_t i = 0; i < data.size(); i++)
   {
	   
	   if (data[i] == '\n')
	   {
		   temp += "\r\n";

		   curDBWindowData.push_back(oneLine);
		   oneLine.clear();
	   }
	   else
	   {
		   temp += data[i];
		   oneLine += data[i];
	   }
	   //insertData((int) i, data[i]);
   }
   SetWindowText(hwnd, temp.c_str() );
	
}	

void MainApp::sendDataToEditWindow(HWND hwnd, vector<string> &data)
{
	HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
	SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"parsed...");
	//SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)dir.c_str());

   string temp = "";
   for(size_t i = 0; i < data.size(); i++)
   {
	   temp += data[i];
	   temp += "\r\n";
	   //insertData((int) i, data[i]);
   }
   SetWindowText(hwnd, temp.c_str() );
	
}

vector<string> MainApp::getDataFromEditWindow(HWND hwnd)
{
	vector<string> returnData;
	HWND hEdit = GetDlgItem(hwnd, IDC_CHILD_EDIT);
	SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 0, (LPARAM)"parsed...");
	//SendDlgItemMessage(g_hMainWindow, IDC_MAIN_STATUS, SB_SETTEXT, 1, (LPARAM)dir.c_str());

	int textCount = GetWindowTextLength(hwnd) + 2;
	char * textBuffer = new char[textCount];
	GetWindowText( hwnd,textBuffer, textCount);

	string temp = "";
	for(size_t i = 0; i < strlen(textBuffer); i++)
	{
		if(textBuffer[i] == '\n')
		{
			returnData.push_back(temp);
			temp = "";
		}
		else
			temp += textBuffer[i];
	}
	if(!temp.empty())//get the last path we created
		returnData.push_back(temp);

	return returnData;
}	


void MainApp::deleteFromFilePathList()
{
	//pathToDB.deleteEmptyDirs(curFileWindowData);
	curFileWindowData.clear();
	sendDataToEditWindow(getFileViewHandle(), curFileWindowData);
}
//this used to be the beggining of how i would read all the text strings (they were paths) in a window, calc some data fron the path, then add to the DB.
//things are differnt now, but i feel ill need this still, so here is the main "logic"

void MainApp::oldGetPathData()
{
	vector<string> data = getDataFromEditWindow(getFileViewHandle());

	if (data.empty())
		curDBWindowData.push_back("nothing to add to DB");

	for (size_t i = 0; i < data.size(); i++)
	{
		/*vector <GalleryData *>d;
		if (!fileWalker->calcGalleryData(data[i], ignorePattern, d))
		{
			string err = "invalid file path: " + data[i];
			curDBWindowData.push_back(err);
			continue;
		}

		for (size_t j = 0; j < d.size(); j++)
		{
		}*/
	}
}