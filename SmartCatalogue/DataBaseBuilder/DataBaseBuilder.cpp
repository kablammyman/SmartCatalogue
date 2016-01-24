// DataBaseBuilder.cpp : Defines the entry point for the console application.
//
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

#include "stdafx.h"
#include "DataBaseBuilder.h"
#include <algorithm>

//check execution time
#include <ctime>

//#define TEST_DATABASE_PARSER 0
//#define TEST_DATABASE_CONTROLLER 1


int main(int argc, char *argv[])
{
	char full[MAX_PATH];
	string temp = _fullpath(full, argv[0], MAX_PATH);
	size_t found = temp.find_last_of("/\\");
	bool verboseOutput = false;
	int goodDir = 0, badDir = 0, totalDir = 0;
	filePathBase = temp.substr(0, found);
	cfgPath = filePathBase + "\\imageViewCfg.txt";
	
	doImageHash = true;

	//read from cfg by default
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
		
	vector<string> dbTableValues = CFG::CFGReaderDLL::getCfgListValue("tableNames");
	//if we cant find the table names in the cfg, thejust get out of here
	if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
	{
		cout << "couldnt find the list of table names in your cfg...\n";
		exit(-1);
	}

	meta = CFG::CFGReaderDLL::getCfgListValue("metaWords");

	//command line args can add extra options
	int i = 0;
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
		else if (strcmp(argv[i], "-ignoreBase") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			ignorePattern = argv[i + 1];
		}
		else if (strcmp(argv[i], "-NoimgHash") == 0)
			doImageHash = false;
		
		else if (strcmp(argv[i], "-verbose") == 0)
			verboseOutput = true;

		i++;

	}

	if (dbPath == "" || pathToProcess == "")
		invalidParmMessageAndExit();

	if (!MyFileDirDll::doesPathExist(pathToProcess))
	{
		cout << pathToProcess << "is not valid\n";
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

	dbCtrlr.openDatabase(dbPath);
	dbDataParser.setDBController(&dbCtrlr);
	if(meta.size() > 0)
		dbDataParser.fillTreeWords(meta);

	if (!dbDataParser.fillPartOfSpeechTable("TableNamesPartOfSceech"))
	{
		cout << "the partsOfSpeechTable is not present in the DB or is not valid\n";
		exit(-1);
	}

	for (size_t i = 0; i < dbTableValues.size(); i++)
		if (!dbDataParser.getDBTableValues(dbTableValues[i]))
		{
			cout << dbTableValues[i] << " is not a valid table in your DB\n";
			exit(-1);
		}

	int start_s = clock();
	//this time, I'm gonna get the data AS i build the dir tree
	MyFileDirDll::startDirTreeStep(pathToProcess);
	

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
		bool ret = dbDataParser.calcGalleryData(curDir, ignorePattern, galleryData, galleryCalcError);
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

			int categoryID = inserCategoryInfoIntoDB(galleryData);
			if (categoryID == -1)
				continue;

			int websiteID = insertWebsiteInfoIntoDB(galleryData, categoryID);
			if (websiteID == -1)
				continue;

			//a zero for subwebsite means there is no subwesite
			int subWebsiteID = insertSubWebsiteInfoIntoDB(galleryData, websiteID);
			if (subWebsiteID == -1)
				continue;

			int galleryID = insertGalleryInfoIntoDB(galleryData, websiteID, subWebsiteID);
			if (galleryID == -1)
				continue;
			
			//gotta fill in meta/keywords...even if they descrivbe models outfit
			for (size_t k = 0; k < galleryData.models.size(); k++)
			{
				if (verboseOutput)
					cout << "model " << k + 1 << " name: " << galleryData.models[k].name.firstName << " "<< galleryData.models[k].name.middleName << " " << galleryData.models[k].name.lastName << endl;

				if(!insertModelInfoIntoDB(galleryData.models[k]))
					continue;
				
				if(!insertModelsInGalleryInfoIntoDB(galleryData.models[k].name.dbID, galleryID))
					continue;

				for (size_t i = 0; i < galleryData.models[k].outfit.size(); i++)
				{
					if(!insertModelOutfitInfoIntoDB(galleryData.models[k], i, galleryID))
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
					//we have this querey seperated from the method, so we dont do un nesc hashing!
					vector<DatabaseController::dbDataPair> imageQuerey;
					imageQuerey.push_back(make_pair("MD5", md5Hash));
					imageQuerey.push_back(make_pair("galleryID", to_string(galleryID)));
					dbCtrlr.doDBQuerey("Images", imageQuerey, output);

					if (!output.empty())
					{
						if (output.find("error") != string::npos)
						{
							string errString = ("image querey error: " + output + "\n");
							cout << errString;
							addEntryToInvalidPathFile(errString);
						}
						continue;//its already been hashed
					}


					string hashingCommand = "\"\"" + filePathBase + "\\CreateImageHash.exe\"  -hash \"";
					hashingCommand += imgeFilePath;
					hashingCommand += "\"\"";

					string hash = exec(hashingCommand.c_str());

					size_t found = hashingCommand.find("-hash");
					hashingCommand.insert(found + 1, "p");
					string phash = exec(hashingCommand.c_str());

					
					if (!insertImageHashInfoIntoDB(*it, hash, phash, md5Hash, galleryID))
					{
						string errString = ("couldnt hash or store: " + imgeFilePath + "\n");
						addEntryToInvalidPathFile(errString);
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
			reportError("invalid directory", curDir, galleryCalcError, true);
		}
		totalDir++;
	}

	int stop_s = clock();
	double milis = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	cout << "done in ";
	printTimeStamp(milis);
	cout << "processed " << totalDir << " galleries\n";
	cout << "\nGalleries added to DB: " << goodDir << " (" <<(goodDir/totalDir)*100<<"%)" <<endl;
	cout << "\nGalleries that dont conform to schema: " << badDir << " (" << (badDir / totalDir) * 100 << "%)" << endl;
	return 0;
}

