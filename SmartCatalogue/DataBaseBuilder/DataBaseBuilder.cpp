// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "DataBaseBuilder.h"
#include <algorithm>

//#define TEST_DATABASE_PARSER 0
//#define TEST_DATABASE_CONTROLLER 1


int main(int argc, char *argv[])
{
	string temp = argv[0];
	size_t found = temp.find_last_of("/\\");

	filePathBase = temp.substr(0, found);
	cfgPath = filePathBase + "\\imageViewCfg.txt";
	int i = 0;
	doImageHash = true;
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

		else if (strcmp(argv[i], "-imgHash") == 0)
		{
			doImageHash = true;
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
		//printf("%s: ", curDir.c_str());
		bool ret = dbDataParser.calcGalleryData(curDir, ignorePattern, galleryData);
		if (ret)
		{
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

			printf("num named models:    %d\n", galleryData.models.size());
			dbGalleryInfo.push_back(make_pair("numModels", to_string(galleryData.models.size())));

			//insert gallery info into DB
			dbCtrlr.insertNewDataEntry("Gallery", dbGalleryInfo, output);

			int galleryID = getLatestID();
				
			for (size_t k = 0; k < galleryData.models.size(); k++)
			{
				//printf("model %d name:", k + 1);
				//printf("%s %s %s\n", galleryData.models[k].name.firstName.c_str(), galleryData.models[k].name.middleName.c_str(), galleryData.models[k].name.lastName.c_str());
				//chyeck to see if we have this model in the DB already, if so, use her ID

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
					//printf("model id in db = %s\n", modelsInDB[0][0].c_str());
					galleryData.models[k].name.dbID = atoi(modelsInDB[0][0].c_str());
				}
				else
				{
					vector<DatabaseController::dbDataPair> dbModelInfo;
					dbModelInfo.push_back(make_pair("firstName", galleryData.models[k].name.firstName));
					dbModelInfo.push_back(make_pair("middleName", galleryData.models[k].name.middleName));
					dbModelInfo.push_back(make_pair("lastName", galleryData.models[k].name.lastName));
					dbCtrlr.insertNewDataEntry("Models", dbModelInfo, output);
												
					galleryData.models[k].name.dbID = getLatestID();
				}

				for (size_t i = 0; i < galleryData.models[k].outfit.size(); i++)
				{
					//printf("%s\n", galleryData.models[k].outfit[0].type.c_str());
					vector<DatabaseController::dbDataPair> dbOutfitInfo;
					dbOutfitInfo.push_back(make_pair("Type", galleryData.models[k].outfit[i].type));
					dbOutfitInfo.push_back(make_pair("ColorID", to_string(galleryData.models[k].outfit[i].ColorIndex)));
					dbOutfitInfo.push_back(make_pair("MaterialID", to_string(galleryData.models[k].outfit[i].ClothingMaterialIndex)));
					dbOutfitInfo.push_back(make_pair("PrintID", to_string(galleryData.models[k].outfit[i].ClothingPrintIndex)));
					dbOutfitInfo.push_back(make_pair("ModelID", to_string(galleryData.models[k].name.dbID)));
					dbOutfitInfo.push_back(make_pair("GalleryID", to_string(galleryID)));
					dbCtrlr.insertNewDataEntry("Outfit", dbOutfitInfo, output);

				}
					
			}
			if (doImageHash)
			{
				list<string> imgFiles = MyFileDirDll::getCurNodeFileList();

				for (list<string>::iterator it = imgFiles.begin(); it != imgFiles.end(); ++it)
				{
					//when the path has spaces,we need DOUBLE quotes aka ""C:\some dir\run.exe"" 
					string fileImg = "\"\"" + filePathBase + "\\CreateImageHash.exe\"  -hash \"" + curDir;
					fileImg += *it;
					fileImg += "\"\"";

					//cout << fileImg << endl;
					string cmdOutput = exec(fileImg.c_str());
					cout << cmdOutput << endl;
				}
			}

			
			//printf("\n");
		}
		//else
		//	printf("its bad!\n");
	}
	return 0;
}

