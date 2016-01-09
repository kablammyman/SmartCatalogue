#pragma once
#include <string>
#include <iostream>

#include "DatabaseDataParser.h"
#include "DatabaseController.h"

#include "CFGReaderDll.h"
#include "myFileDirDll.h"

using namespace std;

string filePathBase;
string cfgPath;
bool doImageHash;

DatabaseDataParser dbDataParser;
DatabaseController dbCtrlr;

string dbPath;
string pathToProcess;
string ignorePattern;//we dont need the base path for all processing, since it wont change


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\\somePath\\\" -dataPath \"C:\\photos\\myFunPhotos\\\"\n";
	exit(-1);
}

int getLatestID()
{
	string output;
	//get the id that was just created from the insert
	dbCtrlr.executeSQL("SELECT last_insert_rowid()", output);
	//othe ouput looks like: last_insert_rowid()|6

	size_t found = output.find_last_of("|");
	string rowID = output.substr(found + 1);
	return atoi(rowID.c_str());
}


string exec(const char* cmd) 
{
	FILE* pipe; 
	pipe = _popen(cmd, "r"); 
	if (!pipe) 
		return "ERROR";
	char buffer[128];
	string result = "";
	
	while (fgets(buffer, 128, pipe) != NULL)
	{
		result += buffer;
	}
	_pclose(pipe);
	return result;
}