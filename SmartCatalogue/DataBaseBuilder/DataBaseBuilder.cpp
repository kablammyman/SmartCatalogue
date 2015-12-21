// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>

#include "DatabaseDataParser.h"
#include "CFGReaderDll.h"

#define TEST_DATABASE_PARSER 1
//#define TEST_DATABASE_CONTROLLER 1

void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\\somePath\\\" -dataPath \"C:\\photos\\myFunPhotos\\\"\n";
	exit(-1);
}

int main(int argc, char *argv[])
{
	string temp = argv[0];
	size_t found = temp.find_last_of("/\\");
	string filePathBase = temp.substr(0, found);
	string cfgPath = filePathBase + "\\imageViewCfg.txt";
	

	DatabaseDataParser dbDataParser;
	DatabaseController dbCtrlr;
	string dbPath;
	string pathToProcess;
	string ignorePattern;//we dont need the base path for all processing, since it wont change
	int i = 0;

	//read from cfg if theres no commandline args
	if (argc < 4)
	{
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
		dbCtrlr.openDatabase(dbPath);
		dbDataParser.setDBController(&dbCtrlr);

		

		dbDataParser.fillPartOfSpeechTable("TableNamesPartOfSceech");
		vector<string> dbTableValues = CFG::CFGReaderDLL::getCfgListValue("tableNames");
		//if we cant find the table names in the cfg, thejust get out of here
		if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
			return -1;
		//else, fill er' up!
		for (size_t i = 0; i < dbTableValues.size(); i++)
			dbDataParser.getDBTableValues(dbTableValues[i]);

		vector<string> meta = CFG::CFGReaderDLL::getCfgListValue("metaWords");
	}
	else
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

#ifdef TEST_DATABASE_PARSER
	//testNamelogic();
	//testSpellchecker();
	dbDataParser.testGalleryCalc();
#endif
#ifdef TEST_DATABASE_CONTROLLER
	testGetTable();
#endif	

	if (pathToProcess != "")
		dbDataParser.getAllPaths(pathToProcess);

	return 0;
}

