#include "stdafx.h"



#include "fileWalker.h"
#include "myFileDirDll.h"

#include <ctime> 
#include <sys/stat.h>


#include <cctype> //isAlpha and the like
#include <algorithm> //for replace method

FileWalker::FileWalker()
{
	

	curPath = "";
	
	
	
	GetSystemTime(&todaysTime);
	//get the day, but not the time...any dir that wa created today should be said is new
	//even if its been added to the DB already...its all good
	todaysTime.wHour = 0;
	todaysTime.wMinute = 0;

}

void FileWalker::takeDirSnapShot(string path, int numThreads, bool refresh)
{
	//ofstream myfile;
	//myfile.open ("emptyDirs.txt");

	if(MyFileDirDll::doesPathExist(path) == false)
		return;
	
	if(curPath == path && !refresh)
		return;

	MyFileDirDll::addDirTree(path, numThreads);
	
	curPath = path;
}
//----------------------------------------------------------------------
void FileWalker::getAllDirPaths(vector<string> &ret)
{
	MyFileDirDll::dumpTreeToVector(curPath, ret, true);
}
//----------------------------------------------------------------------
int FileWalker::getNumFiles()
{
	return MyFileDirDll::getNumFilesInTree(curPath);
}
//----------------------------------------------------------------------
int FileWalker::getNumDirs()
{
	return MyFileDirDll::getNumDirsInTree(curPath);
}

//----------------------------------------------------------------------
void FileWalker::getAllEmptyDirs(vector<string> & emptyDirs)
{
	vector<string> allDirs; 
	MyFileDirDll::dumpTreeToVector(curPath, allDirs, false);
	for(size_t i = 0; i < allDirs.size(); i++)
	{
		if( MyFileDirDll::getNumFilesInDir(allDirs[i]) == 0 &&
			MyFileDirDll::getAllFolderNamesInDir(allDirs[i]).size() ==0 )
		{
			emptyDirs.push_back(allDirs[i]);
		}
	}
}
//----------------------------------------------------------------------
void FileWalker::getAllDirsWithImgs(vector<string> & imgDirs, bool onlyLegalDirs)
{
	vector<string> allDirs; 
	MyFileDirDll::dumpTreeToVector(curPath, allDirs, true);
	for(size_t i = 0; i < allDirs.size(); i++)
	{
		if( MyFileDirDll::getNumFilesInDir(allDirs[i]) != 0)
		{ 
			if(onlyLegalDirs && MyFileDirDll::getAllFolderNamesInDir(allDirs[i]).size() ==0)
				imgDirs.push_back(allDirs[i]);
			else
				imgDirs.push_back(allDirs[i]);
		}
	}
}

//----------------------------------------------------------------------
void FileWalker::deleteEmptyDirs(vector<string> & emptyDirs)
{
	for(size_t i = 0; i < emptyDirs.size(); i++)
		MyFileDirDll::deleteFile(emptyDirs[i]);
}






//when a folder is created or has files added\removed, its creation date is updated
//however moving a folder does not chnage its creation date.
bool FileWalker::isFileNew(string file)
{
	//bool changed = false;
	//DWORD dwAttrib = GetFileAttributes();
	HANDLE hFile = CreateFile(file.c_str(),                // name of the write
						GENERIC_READ,          // open for reading
						1,                      // do share!! (0 = dont share)
						NULL,                   // default security
						OPEN_EXISTING,             
						FILE_FLAG_BACKUP_SEMANTICS,  // normal file
						NULL);   

	if(hFile == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		return false;
	}
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal;
 
	// Retrieve the file times for the file.
	if (!GetFileTime(hFile, &ftCreate, &ftAccess, &ftWrite))
	{
		int err = GetLastError();
		return false;
	}
	// Convert the last-write time to local time.
	FileTimeToSystemTime(&ftWrite, &stUTC);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
 
	CloseHandle(hFile);

	//SetFileAttributes(file.c_str(), dwFileAttributes);
	return shouldFileBeUpdated(&stLocal);

}

bool FileWalker::shouldFileBeUpdated(SYSTEMTIME *creationDate)
{
	if (creationDate->wYear < todaysTime.wYear )
       return false;

	if (creationDate->wMonth < todaysTime.wMonth)
       return false;

	if (creationDate->wDay < todaysTime.wDay)
		return false;
	
	if (creationDate->wHour < todaysTime.wHour)
		return false;
	//if the file was created within the hour, just say it needs updating
	//even if it technically doesnt
	/*if (creationDate->wMinute <= todaysTime)
		return false;*/

	return true;
}

void FileWalker::getTimeSinceLastRun()
{
	time_t deleteTime;
	struct tm* deleteTimeInfo = NULL;
	//used for file comparisions
	time(&deleteTime);
	deleteTimeInfo = localtime ( &deleteTime );


	//delYear = deleteTimeInfo->tm_year;
	//delMon = deleteTimeInfo->tm_mon;
	//delDay = deleteTimeInfo->tm_mday;
}



void FileWalker::GetAllNewDirs()
{
	vector<string> paths;
	getAllDirsWithImgs(paths);

	for (size_t i = paths.size() - 1; i > 0; i--)
	{
		if (!isFileNew(paths[i]))
			paths.erase(paths.begin() + i);
	}

	//get last one
	if (!isFileNew(paths[0]))
		paths.erase(paths.begin());

	//sendDataToEditWindow(getFileViewHandle(), curFileWindowData);
}

