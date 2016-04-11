#include <list>
#include <thread> //for the verify method
#include <algorithm> //for sort and the SET functions

#include "MD5.h"
#include "WinToDBMiddleman.h"
#include "DataBaseBuilder.h"

DatabaseBuilder::DatabaseBuilder(string dbPath,string root)
{
	dbCtrlr.openDatabase(dbPath);
	dbDataParser.setDBController(&dbCtrlr);
	WinToDBMiddleman::SetDBController(&dbCtrlr);
	rootPath = root;
}
//---------------------------------------------------------------------------------------------------------
void DatabaseBuilder::FillMetaWords(vector<string> &meta)
{
	if (meta.size() > 0)
		dbDataParser.fillTreeWords(meta);
}
//---------------------------------------------------------------------------------------------------------
void DatabaseBuilder::FillPartsOfSpeechTable(vector<string> &dbTableValues)
{
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
}
bool DatabaseBuilder::ParsePath(string curDir, GalleryData galleryData, string err)
{
	return dbDataParser.calcGalleryData(curDir, rootPath, galleryData, err);
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::GetLatestID()
{
	string output;
	//get the id that was just created from the insert
	dbCtrlr.executeSQL("SELECT last_insert_rowid()", output);
	//othe ouput looks like: last_insert_rowid()|6

	size_t found = output.find_last_of("|");
	string rowID = output.substr(found + 1);
	return atoi(rowID.c_str());
}


//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::InsertCategoryInfoIntoDB(GalleryData &galleryData)
{
	//add the model to gallery info
	string output;
	vector<DatabaseController::dbDataPair> categoryQuerey;
	categoryQuerey.push_back(make_pair("ID", ""));
	categoryQuerey.push_back(make_pair("name", galleryData.category));
	dbCtrlr.doDBQuerey("Category", categoryQuerey, output);
	//empty output means there name wasnt found
	if (!output.empty())
	{
		if (output.find("error") != string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR,"category querey "+ output + ("category = " + galleryData.category + "\npath = " + galleryData.path));
			return -1;
		}
		vector <vector<string>> categoryInfo; 
		dbCtrlr.parseDBOutput(output, 2, categoryInfo);
		return atoi(categoryInfo[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> categoryInfo;
	categoryInfo.push_back(make_pair("name", galleryData.category));
	dbCtrlr.insertNewDataEntry("Category", categoryInfo, output);

	if (!output.empty())
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "category input error " + output + ("category = " + galleryData.category + "\npath = " + galleryData.path));
		return -1;
	}
	return GetLatestID();
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::InsertWebsiteInfoIntoDB(GalleryData &galleryData, int categoryID)
{
	string output;
	//we dont want the path to the gallery, just to this website only
	string path = (rootPath + galleryData.category + "\\" + galleryData.websiteName);
	//check to see if we have this website in the DB already, if so, the ID

	vector<DatabaseController::dbDataPair> websiteQuerey;
	websiteQuerey.push_back(make_pair("ID", ""));
	websiteQuerey.push_back(make_pair("name", galleryData.websiteName));
	websiteQuerey.push_back(make_pair("path", path));
	dbCtrlr.doDBQuerey("Website", websiteQuerey, output);

	//empty output means there name wasnt found
	if (!output.empty())
	{
		if (output.find("error") != string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "website querey error " + output + ("website = " + galleryData.websiteName + "\npath = " + galleryData.path));
			return -1;
		}
		vector <vector<string>> websiteInfo;
		dbCtrlr.parseDBOutput(output, 2, websiteInfo);
		return atoi(websiteInfo[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> dbWebInfo;

	dbWebInfo.push_back(make_pair("path", path));
	dbWebInfo.push_back(make_pair("name", galleryData.websiteName));
	dbWebInfo.push_back(make_pair("categoryID", to_string(categoryID)));
	dbCtrlr.insertNewDataEntry("Website", dbWebInfo, output);
	if (!output.empty())
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "website input error " + output + ("website = " + galleryData.websiteName + "\npath = " + galleryData.path));
		return -1;
	}
	return GetLatestID();
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::InsertSubWebsiteInfoIntoDB(GalleryData &galleryData, int websiteID)
{
	if (galleryData.subWebsiteName.empty())
		return 0;

	string output;

	//we dont want the path to the gallery, just to this website only
	string path = (rootPath + galleryData.category + "\\" + galleryData.websiteName + "\\" + galleryData.subWebsiteName);

	//check to see if we have this website in the DB already, if so, the ID
	vector<DatabaseController::dbDataPair> subWebsiteQuerey;
	subWebsiteQuerey.push_back(make_pair("ID", ""));
	subWebsiteQuerey.push_back(make_pair("name", galleryData.subWebsiteName));
	subWebsiteQuerey.push_back(make_pair("path", path));
	subWebsiteQuerey.push_back(make_pair("ParentID", to_string(websiteID)));
	dbCtrlr.doDBQuerey("SubWebsite", subWebsiteQuerey, output);

	if (!output.empty())
	{
		if (output.find("error") != string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "subWebsite querey error " + output + ("website = " + galleryData.subWebsiteName + "\npath = " + galleryData.path));
			return -1;
		}
		vector <vector<string>> modelsInDB; 
		dbCtrlr.parseDBOutput(output, 2, modelsInDB);
		return atoi(modelsInDB[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> dbSubWebInfo;
	dbSubWebInfo.push_back(make_pair("path", path));
	dbSubWebInfo.push_back(make_pair("name", galleryData.subWebsiteName));
	dbSubWebInfo.push_back(make_pair("ParentID", to_string(websiteID)));
	dbCtrlr.insertNewDataEntry("SubWebsite", dbSubWebInfo, output);

	if (!output.empty())
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "subWebsite input error " + output + ("website = " + galleryData.subWebsiteName + "\npath = " + galleryData.path));
		return -1;
	}
	return GetLatestID();
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::InsertGalleryInfoIntoDB(GalleryData &galleryData, int websiteID, int subWebsiteID, int categoryID)
{
	string output;

	vector<DatabaseController::dbDataPair> galleryeQuerey;
	galleryeQuerey.push_back(make_pair("ID", ""));
	galleryeQuerey.push_back(make_pair("galleryName", galleryData.galleryName));
	galleryeQuerey.push_back(make_pair("path", galleryData.path));
	dbCtrlr.doDBQuerey("Gallery", galleryeQuerey, output);

	if (!output.empty())
	{
		if (output.find("error") != string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "gallery querey error " + output + ("gallery = " + galleryData.galleryName + "\npath = " + galleryData.path));
			return -1;
		}
		vector <vector<string>> modelsInDB; 
		dbCtrlr.parseDBOutput(output, 2, modelsInDB);
		return atoi(modelsInDB[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> dbGalleryInfo;
	dbGalleryInfo.push_back(make_pair("path", galleryData.path));
	dbGalleryInfo.push_back(make_pair("galleryName", galleryData.galleryName));
	dbGalleryInfo.push_back(make_pair("numModels", to_string(galleryData.models.size())));
	dbGalleryInfo.push_back(make_pair("WebsiteID", to_string(websiteID)));
	dbGalleryInfo.push_back(make_pair("SubWebsiteID", to_string(subWebsiteID)));
	dbGalleryInfo.push_back(make_pair("categoryID", to_string(categoryID)));
	dbCtrlr.insertNewDataEntry("Gallery", dbGalleryInfo, output);
	if (!output.empty())
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "gallery input error " + output + ("gallery = " + galleryData.galleryName + "\npath = " + galleryData.path));
		return -1;
	}
	return GetLatestID();
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::InsertModelInfoIntoDB(GalleryModel &model)
{
	string output;
	//check to see if we have this model in the DB already, if so, use her ID
	vector<DatabaseController::dbDataPair> modelQuerey;
	modelQuerey.push_back(make_pair("ID", ""));
	modelQuerey.push_back(make_pair("firstName", model.name.firstName));
	modelQuerey.push_back(make_pair("middleName", model.name.middleName));
	modelQuerey.push_back(make_pair("lastName", model.name.lastName));
	dbCtrlr.doDBQuerey("Models", modelQuerey, output);

	//empty output means there name wasnt found
	if (!output.empty())
	{
		if (output.find("error") != string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "model querey error " +output+ ("model = " + model.name.firstName + " " + model.name.middleName + " " + model.name.lastName));
			return false;
		}
		vector <vector<string>> modelsInDB; 
		dbCtrlr.parseDBOutput(output, 3, modelsInDB);
		model.name.dbID = atoi(modelsInDB[0][0].c_str());
		return true;
	}

	//insert the new model
	vector<DatabaseController::dbDataPair> dbModelInfo;
	dbModelInfo.push_back(make_pair("firstName", model.name.firstName));
	dbModelInfo.push_back(make_pair("middleName", model.name.middleName));
	dbModelInfo.push_back(make_pair("lastName", model.name.lastName));
	dbCtrlr.insertNewDataEntry("Models", dbModelInfo, output);

	if (!output.empty())
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "model input error " + output + ("model = " + model.name.firstName + " " + model.name.middleName + " " + model.name.lastName));
		return false;
	}

	model.name.dbID = GetLatestID();
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::InsertModelOutfitInfoIntoDB(GalleryModel &model, size_t clothingIndex, int galleryID)
{
	string output;
	vector<DatabaseController::dbDataPair> dbOutfitInfo;
	dbOutfitInfo.push_back(make_pair("Type", model.outfit[clothingIndex].type));
	dbOutfitInfo.push_back(make_pair("TypeID", to_string(model.outfit[clothingIndex].index)));
	dbOutfitInfo.push_back(make_pair("ColorID", to_string(model.outfit[clothingIndex].ColorIndex)));
	dbOutfitInfo.push_back(make_pair("MaterialID", to_string(model.outfit[clothingIndex].ClothingMaterialIndex)));
	dbOutfitInfo.push_back(make_pair("PrintID", to_string(model.outfit[clothingIndex].ClothingPrintIndex)));
	dbOutfitInfo.push_back(make_pair("ModelID", to_string(model.name.dbID)));
	dbOutfitInfo.push_back(make_pair("GalleryID", to_string(galleryID)));
	dbCtrlr.insertNewDataEntry("Outfit", dbOutfitInfo, output);
	if (!output.empty())
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "model outfit input error "+ output+ ("model = " + model.name.firstName + " " + model.name.middleName + " " + model.name.lastName + "\ngalleryID = " + to_string(galleryID)));
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::InsertImageHashInfoIntoDB(string imgeFileName, string hash, string phash, string md5, int galleryID)
{
	string output;
	vector<DatabaseController::dbDataPair> dbImgInfo;
	dbImgInfo.push_back(make_pair("fileName", imgeFileName));
	dbImgInfo.push_back(make_pair("hash", hash));
	dbImgInfo.push_back(make_pair("phash", phash));
	dbImgInfo.push_back(make_pair("MD5", md5));
	dbImgInfo.push_back(make_pair("galleryID", to_string(galleryID)));
	dbCtrlr.insertNewDataEntry("Images", dbImgInfo, output);

	if (!output.empty())
	{
		//if we are trying to input the same image again (lie we restarted the process)
		//then its not an error
		if (output.find("UNIQUE constraint failed") == string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "image hash input error " + output + "filename = " + imgeFileName + "\ngalleryID = " + to_string(galleryID));
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::InsertImageHashInfoIntoDB(string imgeFileName, string md5, int galleryID)
{
	string output;
	vector<DatabaseController::dbDataPair> dbImgInfo;
	dbImgInfo.push_back(make_pair("fileName", imgeFileName));
	dbImgInfo.push_back(make_pair("MD5", md5));
	dbImgInfo.push_back(make_pair("galleryID", to_string(galleryID)));
	dbCtrlr.insertNewDataEntry("Images", dbImgInfo, output);

	if (!output.empty())
	{
		//if we are trying to input the same image again (lie we restarted the process)
		//then its not an error
		if (output.find("UNIQUE constraint failed") == string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "image hash input error "+ output+ "filename = " + imgeFileName + "\ngalleryID = " + to_string(galleryID));
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::AddHashDataToDB(string imgeFileName, string hash, string phash)
{
	string output;
	vector<DatabaseController::dbDataPair> dbImgInfo;

	string fileName = MyFileDirDll::getFileNameFromPathString(imgeFileName);

	vector<DatabaseController::dbDataPair> imgQuerey;
	imgQuerey.push_back(make_pair("fileName", fileName));
	dbCtrlr.doDBQuerey("Images", imgQuerey, output);

	//empty output means there name wasnt found
	if (!output.empty())
	{
		if (output.find("error") != string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "image hash lookup error " + output + "couldnt find: " + imgeFileName + " in DB to add phash\n");
			return false;
		}
	}

	dbImgInfo.push_back(make_pair("hash", hash));
	dbImgInfo.push_back(make_pair("phash", phash));
	dbCtrlr.UpdateEntry("Images", dbImgInfo, make_pair("fileName", fileName), output);

	if (!output.empty())
	{
		//if we are trying to input the same image again (like we restarted the process)
		//then its not an error
		if (output.find("UNIQUE constraint failed") == string::npos)
		{
			logger->WriteMessage(LOG_LEVEL_ERROR, "image hash input error "+ output+ "filename = " + fileName);
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::InsertModelsInGalleryInfoIntoDB(int modelID, int galleryID)
{
	//add the model to gallery info
	string output;
	vector<DatabaseController::dbDataPair> modelsInGalleryInfo;
	modelsInGalleryInfo.push_back(make_pair("modelID", to_string(modelID)));
	modelsInGalleryInfo.push_back(make_pair("galleryID", to_string(galleryID)));
	dbCtrlr.insertNewDataEntry("ModelsInGallery", modelsInGalleryInfo, output);

	if (!output.empty())
		return false;
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::IsImageInDB(int galleryID, string md5Hash)
{
	//we have this querey seperated from the method, so we dont do un nesc hashing!
	string output;
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
			logger->WriteMessage(LOG_LEVEL_ERROR, errString);
		}
		return true;
	}
	return false;
}
bool DatabaseBuilder::VerifyImage(string imagePath, int &galleryID)
{
	string md5Hash = createMD5Hash(imagePath);
	string galleryPath = MyFileDirDll::getPathFromFullyQualifiedPathString(imagePath);
	galleryID = WinToDBMiddleman::GetGalleryIDFromPath(galleryPath);
	if(!IsImageInDB(galleryID, md5Hash))
		return false;
	return true;
}
void DatabaseBuilder::VerifyDB(string root)
{
	DatabaseDataParser dbParse;
	vector<string> treeOnDisk;
	vector<string> dbPAths;
	
	//[] is the capture list, add vars that are inside the lambda here

	thread getDBSnapshot([this, root, &dbPAths]()
	{
		//now get all the galleries in the currentDB
		string querey, output;
		querey = ("SELECT path FROM Gallery WHERE path LIKE \"" + root + "%\"");
		dbCtrlr.executeSQL(querey, output);
		dbCtrlr.getAllValuesFromCol(output,"path", dbPAths);
		//for testing
		/*for (size_t i = 0; i < dbPAths.size(); i++)
		{
			//SERVER = 8 F:\ = 3
			if(dbPAths[1][0] == 'F')
				dbPAths[i].replace(0,3, "\\\\SERVER\\");
		}*/
		sort(dbPAths.begin(), dbPAths.end());
	});

	
	thread getDiskSnapshot([this, &treeOnDisk, &root]()
	{
		//root.replace(0, 3, "\\\\SERVER\\");
		//get all paths on disk that have images only
		dbDataParser.getAllPaths(root, treeOnDisk, 10,true);
		sort(treeOnDisk.begin(), treeOnDisk.end());
	});

	// synchronize threads, so we can compare the data
	getDiskSnapshot.join();  
	getDBSnapshot.join(); 

	vector<string>::iterator it;
	vector<string> commonDirs(treeOnDisk.size());
	it = set_intersection(treeOnDisk.begin(), treeOnDisk.end(), dbPAths.begin(), dbPAths.end(), commonDirs.begin());

	commonDirs.resize(it - commonDirs.begin());
	string intersectionStr = ("The common dirs " + to_string(commonDirs.size()) + " elements\n");
	logger->WriteMessage(LOG_LEVEL_DEBUG,intersectionStr);

	vector<string> dirsToDeleteFromDB(dbPAths.size());
	it = set_difference(dbPAths.begin(), dbPAths.end(), commonDirs.begin(), commonDirs.end(), dirsToDeleteFromDB.begin());

	dirsToDeleteFromDB.resize(it - dirsToDeleteFromDB.begin());
	intersectionStr = ("The delete dirs " + to_string(dirsToDeleteFromDB.size()) + " elements\n");
	logger->WriteMessage(LOG_LEVEL_DEBUG, intersectionStr);

	string output;
	int counter = 0;
	for (size_t i = 0; i < dirsToDeleteFromDB.size(); i++)
	{
		if (!WinToDBMiddleman::DeleteGalleryFromDB(dirsToDeleteFromDB[i], output))
			logger->WriteMessage(LOG_LEVEL_ERROR,"deleteing from db error: "+ output + " for " + dirsToDeleteFromDB[i]);
		else
			counter++;
	}

	string removeMsg = ("removed " + to_string(counter) + " galleries from DB");
	logger->WriteMessage(LOG_LEVEL_DEBUG, removeMsg);
	
	vector<string> dirsNotInDB(treeOnDisk.size());
	it = set_difference(treeOnDisk.begin(), treeOnDisk.end(), commonDirs.begin(), commonDirs.end(), dirsNotInDB.begin());

	dirsNotInDB.resize(it - dirsNotInDB.begin());
	
	counter = 0;
	for (size_t i = 0; i < dirsNotInDB.size(); i++)
	{
		if (!AddDirToDB(dirsNotInDB[i], true))
			logger->WriteMessage(LOG_LEVEL_ERROR, "adding to db error: " + output + " for " + dirsNotInDB[i]);
		else
			counter++;
	}

	string addMsg = ("added " + to_string(counter) + " galleries to DB");
	logger->WriteMessage(LOG_LEVEL_DEBUG, addMsg); 
}

bool DatabaseBuilder::AddDirToDB(string curDir, bool doImageHash)
{
	if (verboseOutput)
		cout << curDir;

	//if the current node has folders in it, then it should NOT have images. even if it does, those images
	//will be ignored since its not following the protocol
	if (MyFileDirDll::getNumFoldersinDir(curDir) > 0)
		return false;

	GalleryData galleryData;
	string galleryCalcError;
	bool ret = dbDataParser.calcGalleryData(curDir, rootPath, galleryData, galleryCalcError);
	if (!ret)
	{
		logger->WriteMessage(LOG_LEVEL_ERROR, "invalid directory "+ curDir+ galleryCalcError);
		return false;
	}

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

	int categoryID = InsertCategoryInfoIntoDB(galleryData);
	if (categoryID == -1)
		return false;

	int websiteID = InsertWebsiteInfoIntoDB(galleryData, categoryID);
	if (websiteID == -1)
		return false;

	//a zero for subwebsite means there is no subwesite
	int subWebsiteID = InsertSubWebsiteInfoIntoDB(galleryData, websiteID);
	if (subWebsiteID == -1)
		return false;

	int galleryID = InsertGalleryInfoIntoDB(galleryData, websiteID, subWebsiteID, categoryID);
	if (galleryID == -1)
		return false;

	//gotta fill in meta/keywords...even if they descrivbe models outfit
	for (size_t k = 0; k < galleryData.models.size(); k++)
	{
		if (verboseOutput)
			cout << "model " << k + 1 << " name: " << galleryData.models[k].name.firstName << " " << galleryData.models[k].name.middleName << " " << galleryData.models[k].name.lastName << endl;

		if (!InsertModelInfoIntoDB(galleryData.models[k]))
			return false;

		if (!InsertModelsInGalleryInfoIntoDB(galleryData.models[k].name.dbID, galleryID))
			return false;

		for (size_t i = 0; i < galleryData.models[k].outfit.size(); i++)
		{
			if (!InsertModelOutfitInfoIntoDB(galleryData.models[k], i, galleryID))
				return false;
		}
	}

	return true;
}

bool DatabaseBuilder::RequestImageHashesForDir(string galleryPath,int galleryID, NetworkConnection *conn, int connIndex)
{
	vector<string> imgFiles = MyFileDirDll::getAllFileNamesInDir(galleryPath);

	for (size_t i = 0; i < imgFiles.size(); i++)
	{ 
		string imgeFilePath = (galleryPath + imgFiles[i]);
		string md5Hash = createMD5Hash(imgeFilePath);

		if (IsImageInDB(galleryID, md5Hash))
			continue;

		if (!InsertImageHashInfoIntoDB(imgFiles[i], md5Hash, galleryID))
			continue;

		//createImageHash should send back the hashes and the file path they are made from
		//when the data comes back, we will add the hash/phash info
		string hashingCommand = ("-allHash," + imgeFilePath);
		conn->sendData(connIndex, hashingCommand.c_str());
	}
	return true;
}
//since the dir watcher gets all the images we add, this makes more sense
bool DatabaseBuilder::RequestImageHash(string imgPath, int galleryID, NetworkConnection *conn, int connIndex)
{
	string md5Hash = createMD5Hash(imgPath);

	if (IsImageInDB(galleryID, md5Hash))
		return false;
	
	if (!InsertImageHashInfoIntoDB(MyFileDirDll::getFileNameFromPathString(imgPath), md5Hash, galleryID))
		return false;

	//createImageHash should send back the hashes and the file path they are made from
	//when the data comes back, we will add the hash/phash info
	string hashingCommand = ("-allHash," + imgPath);
	conn->sendData(connIndex, hashingCommand.c_str());
	
	return true;
}