// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DatabaseDataParser.h"
/*
void processPath(std::string path="");

void MainApp::processPath(string path)
{
if(path != "")
dir = path;
pathToDB.getAllPaths(dir);
}

*/

int main()
{
	DatabaseDataParser pathToDB;

	/*dbCtrlr.openDatabase(CFG::CFGReaderDLL::getCfgStringValue("DBPath"));
	pathToDB.setDBController(&dbCtrlr);
	
	vector<string> meta = CFG::CFGReaderDLL::getCfgListValue("metaWords");
	
	
	ignorePattern = CFG::CFGReaderDLL::getCfgStringValue("ignorePattern");
	vector<string> dbTableValues = CFG::CFGReaderDLL::getCfgListValue("tableNames");

	//if we cant find the table names in the cfg, thejust get out of here
	if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
		return;

	for (size_t i = 0; i < dbTableValues.size(); i++)
		pathToDB.getDBTableValues(dbTableValues[i]);
	*/

	return 0;
}

