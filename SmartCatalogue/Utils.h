#pragma once

#include <string>
#include <Windows.h>
#include "CFGReaderDll.h"

using namespace std;

string filePathBase;
string dbPath;
string pathToProcess;
string ignorePattern;

string getProgramPath(char *argv)
{
	char full[MAX_PATH];
	string temp = _fullpath(full, argv, MAX_PATH);
	size_t found = temp.find_last_of("/\\");
	return temp.substr(0, found);
}

void loadCFGFile(string programBasePath)
{
	string cfgPath = filePathBase + "\\imageViewCfg.txt";

	if (!CFG::CFGReaderDLL::readCfgFile(cfgPath, '|'))
	{
		string errorMsg = "Error opening :";
		errorMsg += cfgPath;
		cout << errorMsg << "\nno cfg text file and...\n";
		invalidParmMessageAndExit();
	}

	dbPath = CFG::CFGReaderDLL::getCfgStringValue("DBPath");
	pathToProcess = CFG::CFGReaderDLL::getCfgStringValue("mainWorkingPath");
	ignorePattern = CFG::CFGReaderDLL::getCfgStringValue("ignorePattern");
}