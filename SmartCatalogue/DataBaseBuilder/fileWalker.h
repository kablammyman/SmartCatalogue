#ifndef FILEWALKER_H
#define FILEWALKER_H

#include "windows.h"

#include <iostream>
#include <fstream>

#include <string>
#include <vector> 


using namespace std;



class FileWalker
{
private:
	
	string curPath;
	SYSTEMTIME todaysTime;

	SYSTEMTIME startTime;
	vector<string> curDirTreeVec;
	

public:
	FileWalker();
	void takeDirSnapShot(string path, bool refresh = false);
	
	void getAllDirPaths(vector<string> &ret);
	void getAllEmptyDirs(vector<string> & emptyDirs);

	int getNumFiles();
	int getNumDirs();
	
	void deleteEmptyDirs(vector<string> & emptyDirs);

	
	void getAllDirsWithImgs(vector<string> & imgDirs, bool onlyLegalDirs = true);

	bool isFileNew(string file);
	bool shouldFileBeUpdated (SYSTEMTIME *creationDate);
	void getTimeSinceLastRun();

	void GetAllNewDirs();
};
#endif //FILEWALKER_H