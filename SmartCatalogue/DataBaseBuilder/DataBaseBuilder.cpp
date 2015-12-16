// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include "DatabaseDataParser.h"

void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\somePath\" -dataPath \"C:\photos\myFunPhotos\"\n";
	exit(1);
}

int main(int argc, char *argv[])
{
	DatabaseDataParser dbDataParser;
	DatabaseController dbCtrlr;
	string dbPath;
	string pathToProcess;
	string ignorePattern;//we dont need the base path for all processing, since it wont change
	int i = 0;

	if (argc < 4)
		invalidParmMessageAndExit();
	
	while (i < argc)
	{
		if (strcmp(argv[i], "-dbPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			dbPath = argv[i + 1];
		}
		else if (strcmp(argv[i], "-dataPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			pathToProcess = argv[i + 1];
			ignorePattern = pathToProcess;
		}
		//optional param
		else if (strcmp(argv[i], "-ignoreBase") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			ignorePattern = argv[i + 1];
		}
		i++;
	}

	if (dbPath == "" || pathToProcess == "")
		invalidParmMessageAndExit();

	dbCtrlr.openDatabase(dbPath);
	dbDataParser.setDBController(&dbCtrlr);
	
	if (pathToProcess != "")
		dbDataParser.getAllPaths(pathToProcess);


	/*vector<string> meta = CFG::CFGReaderDLL::getCfgListValue("metaWords");
	
	
	
	vector<string> dbTableValues = CFG::CFGReaderDLL::getCfgListValue("tableNames");

	//if we cant find the table names in the cfg, thejust get out of here
	if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
		return;

	for (size_t i = 0; i < dbTableValues.size(); i++)
		pathToDB.getDBTableValues(dbTableValues[i]);
	*/

	return 0;
}

