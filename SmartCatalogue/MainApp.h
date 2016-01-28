#ifndef MAINAPP_H
#define MAINAPP_H

#include <windows.h>
#include <commctrl.h>
#include <stdio.h>


#include "DatabaseController.h"


//read this one day to fully implement c++
//http://www.infernodevelopment.com/c-win32-api-simple-gui-wrapper

class MainApp
{
	
	MainApp() {};
	MainApp(MainApp const&) {};             // copy constructor is private
	MainApp& operator=(MainApp const&) {};  // assignment operator is private
	static MainApp* m_pInstance;

	FILE *pFile;

	time_t rawtime;
	struct tm* timeinfo;

	struct tm* creationTime;     
	struct stat attrib;         // create a file attribute structure
	
	
	

	HWND g_hMDIDBView;
	HWND g_hMDIFileView;
	HWND g_hMainWindow;
	HINSTANCE hInst;	
	string szWindowClass;
	string szChildWindowClassDB;
	string szChildWindowClassDir;
	vector<string> dontDeleteList;
	
	vector<string> curFileWindowData;
	
public: 
	vector<string> curDBWindowData;
	static MainApp* Instance();

	DatabaseController dbCtrlr;
	
	int windowWidth;
	int windowHeight;
	int buttonW,buttonH;
	int baseX, baseY;

	int winWidth;
	int winHeight;

	int dbWindowX,dbWindowY, fileWindowX,fileWindowY;
	string dir;
	string mainWorkingPath;
	
	string ignorePattern;
	string curDBCommand;
	
	void initVars();
		
	void calcWindowSize();

	void setMainWindow(HWND hwnd);

	void setInstance(HINSTANCE i);
	HINSTANCE getInstance();

	const char *getClassName();
	
	HWND getMainWindowHandle();
	void setMainWindowHandle(HWND hwnd);

	HWND getDBViewHandle();
	void setDBViewHandle(HWND hwnd);

	HWND getFileViewHandle();
	void setFileViewHandle(HWND hwnd);


	void DoFileOpen(HWND hwnd, std::string &name);
	void DoFileSave(HWND hwnd, std::string &name);
	std::string DoBrowseFolder(HWND hwnd);

		
	void callCreateNewDatabase();
	void sendDataToEditWindow(HWND hwnd, string data);
	void sendDataToEditWindow(HWND hwnd, vector<string> &data);
	vector<string> getDataFromEditWindow(HWND hwnd);

	void deleteFromFilePathList();

	void oldGetPathData();
};
#endif //MAINAPP_H