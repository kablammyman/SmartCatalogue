// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>

#include "DatabaseDataParser.h"
#include "DatabaseController.h"

#include "CFGReaderDll.h"
#include "myFileDirDll.h"

//#define TEST_DATABASE_PARSER 0
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
	dbCtrlr.testGetTable();
	dbCtrlr.testDBEntry();
	dbCtrlr.testDBQuerey();
#endif	

	/*// old and inefficnet way
	//becasue i do 1 treversal to create the tree, then another to process the data i need
	vector<string> imageDirs;
	//get all the paths and put the info in memory
	if (pathToProcess != "")
		dbDataParser.getAllPaths(pathToProcess, imageDirs);
	for (size_t i = 0; i < imageDirs.size(); i++)
		dbDataParser.calcGalleryData(imageDirs[i], ignorePattern, galleryData);
		etc etc etc
	*/

	//this time, I'm gonna get the data AS i build the dir tree
	MyFileDirDll::startDirTreeStep(pathToProcess);

	while(!MyFileDirDll::isFinished())
	{
		string curDir = MyFileDirDll::nextDirTreeStep();
		
		//if the current node has folders in it, then it should NOT have images. even if it does, those images
		//will be ignored since its not following the protocol
		if (MyFileDirDll::getCurNodeNumFolders() > 0)
			continue;

		GalleryData galleryData;
		printf("%s: ", curDir.c_str());
		bool ret = dbDataParser.calcGalleryData(curDir, ignorePattern, galleryData);
		if (ret)
		{
			printf("\n");

			if (galleryData.models.size() > 0)
			{
				
				for (size_t k = 0; k < galleryData.models.size(); k++)
				{
					printf("model %d name:", k + 1);
					printf("%s %s %s\n", galleryData.models[k].name.firstName.c_str(), galleryData.models[k].name.middleName.c_str(), galleryData.models[k].name.lastName.c_str());
					//chyeck to see if we have this model in the DB already, if so, use her ID
					string output;
					vector<DatabaseController::dbDataPair> data;
					data.push_back(make_pair("ID", ""));
					data.push_back(make_pair("firstName", galleryData.models[k].name.firstName));
					data.push_back(make_pair("middleName", galleryData.models[k].name.middleName));
					data.push_back(make_pair("lastName", galleryData.models[k].name.lastName));
					dbCtrlr.doDBQuerey("Models", data, output);

					//empty output means there name wasnt found
					if (!output.empty())
					{
						vector <vector<string>> modelsInDB = dbDataParser.parseDBOutput(output, 3);
						printf("model id in db = %s\n", modelsInDB[0][0].c_str());
						galleryData.models[k].name.dbID = atoi(modelsInDB[0][0].c_str());
					}
					else
					{
						vector<DatabaseController::dbDataPair> dbModelInfo;
						dbModelInfo.push_back(make_pair("firstName", galleryData.models[k].name.firstName));
						dbModelInfo.push_back(make_pair("middleName", galleryData.models[k].name.middleName));
						dbModelInfo.push_back(make_pair("lastName", galleryData.models[k].name.lastName));
						dbCtrlr.insertNewDataEntry("Models", dbModelInfo, output);


						dbModelInfo.push_back(make_pair("ID", ""));
						dbCtrlr.doDBQuerey("Models", dbModelInfo, output);
						vector <vector<string>> modelsInDB = dbDataParser.parseDBOutput(output, 4);
						galleryData.models[k].name.dbID = atoi(modelsInDB[0][0].c_str());
					}
					if (galleryData.models[k].outfit.size() > 0)
					{
						printf("model %d first outfit:", k + 1);
						printf("%s\n", galleryData.models[k].outfit[0].type.c_str());
						/*printf("%d\n", galleryData.models[k].sexActionIndex);
						printf("%d\n", galleryData.models[k].hairColorIndex);*/
					}
				}
			}
			string output;
			vector<DatabaseController::dbDataPair> dbGalleryInfo;

			printf("path:       %s\n", galleryData.path.c_str());
			dbGalleryInfo.push_back(make_pair("path", galleryData.category));

			printf("websiteName:    %s\n", galleryData.websiteName.c_str());
			dbGalleryInfo.push_back(make_pair("websiteName", galleryData.websiteName));

			printf("subWebsiteName: %s\n", galleryData.subWebsiteName.c_str());
			dbGalleryInfo.push_back(make_pair("subWebsiteName", galleryData.subWebsiteName));

			printf("galleryName:    %s\n", galleryData.galleryName.c_str());
			dbGalleryInfo.push_back(make_pair("galleryName", galleryData.galleryName));
			
			//insert gallery info into DB
			dbCtrlr.insertNewDataEntry("Gallery", dbGalleryInfo, output);

			//how do i get the model id and the gallery id?


			//printf("%s\n", galleryData.metaData.c_str());
			printf("\n");
		}
		else
			printf("its bad!\n");
	}
	return 0;
}

