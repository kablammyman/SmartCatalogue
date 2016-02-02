// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "DataBaseManager.h"
#include "DataBaseBuilder.h"
#include "CFGReaderDll.h"
#include <algorithm>
#include "Utils.h"
#include <ctime>//check execution time

//#define TEST_DATABASE_PARSER 0
//#define TEST_DATABASE_CONTROLLER 1


int main(int argc, char *argv[])
{
	bool verboseOutput = false;
	int goodDir = 0, badDir = 0, totalDir = 0;
	doImageHash = true;

	Utils::setProgramPath(argv[0]);
	Utils::loadCFGFile();
		
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

			Utils::dbPath = argv[i];
		}
		else if (strcmp(argv[i], "-dataPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			Utils::pathToProcess = argv[i];
		}
		else if (strcmp(argv[i], "-ignoreBase") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			Utils::ignorePattern = argv[i];
		}
		else if (strcmp(argv[i], "-NoimgHash") == 0)
			doImageHash = false;
		
		else if (strcmp(argv[i], "-verbose") == 0)
			verboseOutput = true;

		i++;

	}

	if (Utils::dbPath == "" || Utils::pathToProcess == "")
		invalidParmMessageAndExit();

	if (!MyFileDirDll::doesPathExist(Utils::pathToProcess))
	{
		cout << Utils::pathToProcess << "is not valid\n";
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

	int start_s = clock();
	
	//get the data AS i build the dir tree
	MyFileDirDll::startDirTreeStep(Utils::pathToProcess);
	DatabaseBuilder dbBuilder(Utils::dbPath, Utils::ignorePattern);
	dbBuilder.fillMetaWords(Utils::meta);
	dbBuilder.fillPartsOfSpeechTable(dbTableValues);

	while(!MyFileDirDll::isFinished())
	{
		string curDir = MyFileDirDll::nextDirTreeStep();
		
		//if the current node has folders in it, then it should NOT have images. even if it does, those images
		//will be ignored since its not following the protocol
		if (MyFileDirDll::getCurNodeNumFolders() > 0)
			continue;

		
		if (verboseOutput)
			cout << curDir;

		GalleryData galleryData;
		string galleryCalcError;
		bool ret = dbBuilder.parsePath(curDir, galleryData, galleryCalcError);
		if (ret)
		{
			if (verboseOutput)
				cout << endl;

			string output;
			
			if (verboseOutput)
			{
				cout << "catogory:       " << galleryData.category << endl;
				cout << "websiteName:    " << galleryData.websiteName << endl;
				cout << "subWebsiteName: " << galleryData.subWebsiteName << endl;
				cout << "galleryName:    " << galleryData.galleryName << endl;
				cout << "named   models: " << galleryData.models.size() << endl;
			}

			int categoryID = dbBuilder.inserCategoryInfoIntoDB(galleryData);
			if (categoryID == -1)
				continue;

			int websiteID = dbBuilder.insertWebsiteInfoIntoDB(galleryData, categoryID);
			if (websiteID == -1)
				continue;

			//a zero for subwebsite means there is no subwesite
			int subWebsiteID = dbBuilder.insertSubWebsiteInfoIntoDB(galleryData, websiteID);
			if (subWebsiteID == -1)
				continue;

			int galleryID = dbBuilder.insertGalleryInfoIntoDB(galleryData, websiteID, subWebsiteID, categoryID);
			if (galleryID == -1)
				continue;
			
			//gotta fill in meta/keywords...even if they descrivbe models outfit
			for (size_t k = 0; k < galleryData.models.size(); k++)
			{
				if (verboseOutput)
					cout << "model " << k + 1 << " name: " << galleryData.models[k].name.firstName << " "<< galleryData.models[k].name.middleName << " " << galleryData.models[k].name.lastName << endl;

				if(!dbBuilder.insertModelInfoIntoDB(galleryData.models[k]))
					continue;
				
				if(!dbBuilder.insertModelsInGalleryInfoIntoDB(galleryData.models[k].name.dbID, galleryID))
					continue;

				for (size_t i = 0; i < galleryData.models[k].outfit.size(); i++)
				{
					if(!dbBuilder.insertModelOutfitInfoIntoDB(galleryData.models[k], i, galleryID))
						continue;
				}		
			}

			if (doImageHash)
			{
				list<string> imgFiles = MyFileDirDll::getCurNodeFileList();

				for (list<string>::iterator it = imgFiles.begin(); it != imgFiles.end(); ++it)
				{
					//when the path has spaces,we need DOUBLE quotes aka ""C:\some dir\run.exe"" 
					string imgeFilePath = (curDir+ *it);
					string md5Hash = createMD5Hash(imgeFilePath);
					
					if (dbBuilder.isImageInDB(galleryID, md5Hash))
						continue;


					string hashingCommand = "\"\"" + Utils::filePathBase + "\\CreateImageHash.exe\"  -hash \"";
					hashingCommand += imgeFilePath;
					hashingCommand += "\"\"";

					string hash = exec(hashingCommand.c_str());

					size_t found = hashingCommand.find("-hash");
					hashingCommand.insert(found + 1, "p");
					string phash = exec(hashingCommand.c_str());

					
					if (!dbBuilder.insertImageHashInfoIntoDB(*it, hash, phash, md5Hash, galleryID))
					{
						string errString = ("couldnt hash or store: " + imgeFilePath + "\n");
						dbBuilder.addEntryToInvalidPathFile(errString);
						if (verboseOutput)
							cout << errString;
					}
						
				}
			}
			goodDir++;
		}
		else
		{
			badDir++;
			dbBuilder.reportError("invalid directory", curDir, galleryCalcError, true);
		}
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

