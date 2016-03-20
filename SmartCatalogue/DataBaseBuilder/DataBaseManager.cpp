// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "DataBaseManager.h"
#include "DataBaseBuilder.h"
#include "CFGReaderDll.h"
#include <algorithm>
#include "Utils.h"
#include "CFGHelper.h"

#include <ctime>//check execution time

//#define TEST_DATABASE_PARSER 0
//#define TEST_DATABASE_CONTROLLER 1


int main(int argc, char *argv[])
{
	bool verboseOutput = false;
	int goodDir = 0, badDir = 0, totalDir = 0;
	doImageHash = true;

	CFGHelper::filePathBase = Utils::setProgramPath(argv[0]);
	CFGHelper::loadCFGFile();
		
	vector<string> dbTableValues = CFG::CFGReaderDLL::getCfgListValue("tableNames");
	//if we cant find the table names in the cfg, thejust get out of here
	if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
	{
		cout << "couldnt find the list of table names in your cfg...\n";
		exit(-1);
	}

	

	//command line args can add extra options
	int i = 0;
	while (i < argc)
	{
		if (strcmp(argv[i], "-dbPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			CFGHelper::dbPath = argv[i];
		}
		else if (strcmp(argv[i], "-dataPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			CFGHelper::pathToProcess = argv[i];
		}
		else if (strcmp(argv[i], "-ignoreBase") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			CFGHelper::ignorePattern = argv[i];
		}
		else if (strcmp(argv[i], "-NoimgHash") == 0)
			doImageHash = false;
		
		else if (strcmp(argv[i], "-verbose") == 0)
			verboseOutput = true;

		i++;

	}

	if (CFGHelper::dbPath == "" || CFGHelper::pathToProcess == "")
		invalidParmMessageAndExit();

	if (!MyFileDirDll::doesPathExist(CFGHelper::pathToProcess))
	{
		cout << CFGHelper::pathToProcess << "is not valid\n";
		exit(-1);
	}

#ifdef TEST_DATABASE_PARSER
	//testNamelogic();
	//testSpellchecker();
	dbDataParser.testGalleryCalc();
#endif
#ifdef TEST_DATABASE_CONTROLLER
	dbCtrlr.testGetTable();
	dbCtrlr.testDBEntry();
	dbCtrlr.testDBQuerey();
#endif	

	DatabaseBuilder dbBuilder(CFGHelper::dbPath, CFGHelper::ignorePattern);
	dbBuilder.fillMetaWords(CFGHelper::meta);
	dbBuilder.fillPartsOfSpeechTable(dbTableValues);
	//dbBuilder.verifyDB(CFGHelper::pathToProcess);

	int start_s = clock();
	//get the data AS i build the dir tree
	MyFileDirDll::startDirTreeStep(CFGHelper::pathToProcess);
	while(!MyFileDirDll::isFinished())
	{
		string curDir = MyFileDirDll::nextDirTreeStep();
		
		if (dbBuilder.addDirToDB(curDir))
			goodDir++;
		else
			badDir++;
	
		totalDir++;
	}

	int stop_s = clock();
	double milis = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "done in "<< Utils::getTimeStamp(milis) << endl;
	cout << "processed " << totalDir << " galleries\n";
	float goodPercent = (goodDir / totalDir);
	goodPercent *= 100;
	cout << "\nGalleries added to DB: " << goodDir << " (" << goodPercent <<"%)" <<endl;
	float badPercent = (badDir / totalDir);
	badPercent *= 100;
	cout << "\nGalleries that dont conform to schema: " << badDir << " (" << badPercent << "%)" << endl;
	return 0;
}

