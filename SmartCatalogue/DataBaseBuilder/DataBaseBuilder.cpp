#include "DataBaseBuilder.h"

DatabaseBuilder::DatabaseBuilder(string dbPath,string root)
{
	dbCtrlr.openDatabase(dbPath);
	dbDataParser.setDBController(&dbCtrlr);
	rootPath = root;
}
//---------------------------------------------------------------------------------------------------------
void DatabaseBuilder::fillMetaWords(vector<string> &meta)
{
	if (meta.size() > 0)
		dbDataParser.fillTreeWords(meta);
}
//---------------------------------------------------------------------------------------------------------
void DatabaseBuilder::fillPartsOfSpeechTable(vector<string> &dbTableValues)
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
bool DatabaseBuilder::parsePath(string curDir, GalleryData galleryData, string err)
{
	return dbDataParser.calcGalleryData(curDir, rootPath, galleryData, err);
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::getLatestID()
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
int DatabaseBuilder::addEntryToInvalidPathFile(string message)
{
	FILE * pFile;
	pFile = fopen("invalidPaths.txt", "a");
	if (pFile == NULL)
		return -1;

	fputs(message.c_str(), pFile);
	fclose(pFile);

	return 0;
}

//---------------------------------------------------------------------------------------------------------
void DatabaseBuilder::reportError(string erreorType, string errorMessage, string extraInfo, bool outputToLog)
{
	string msg = (erreorType + ": " + errorMessage);

	if (!extraInfo.empty())
		msg += ("\n" + extraInfo);

	msg += "\n";
	if (outputToLog)
		addEntryToInvalidPathFile(msg);

	cout << msg;
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::inserCategoryInfoIntoDB(GalleryData &galleryData)
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
			reportError("category querey error", output, ("category = " + galleryData.category + "\npath = " + galleryData.path), false);
			return -1;
		}
		vector <vector<string>> categoryInfo; 
		dbDataParser.parseDBOutput(output, 2, categoryInfo);
		return atoi(categoryInfo[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> categoryInfo;
	categoryInfo.push_back(make_pair("name", galleryData.category));
	dbCtrlr.insertNewDataEntry("Category", categoryInfo, output);

	if (!output.empty())
	{
		reportError("category input error", output, ("category = " + galleryData.category + "\npath = " + galleryData.path), false);
		return -1;
	}
	return getLatestID();
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::insertWebsiteInfoIntoDB(GalleryData &galleryData, int categoryID)
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
			reportError("website querey error", output, ("website = " + galleryData.websiteName + "\npath = " + galleryData.path), false);
			return -1;
		}
		vector <vector<string>> websiteInfo;
		dbDataParser.parseDBOutput(output, 2, websiteInfo);
		return atoi(websiteInfo[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> dbWebInfo;

	dbWebInfo.push_back(make_pair("path", path));
	dbWebInfo.push_back(make_pair("name", galleryData.websiteName));
	dbWebInfo.push_back(make_pair("categoryID", to_string(categoryID)));
	dbCtrlr.insertNewDataEntry("Website", dbWebInfo, output);
	if (!output.empty())
	{
		reportError("website input error", output, ("website = " + galleryData.websiteName + "\npath = " + galleryData.path), false);
		return -1;
	}
	return getLatestID();
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::insertSubWebsiteInfoIntoDB(GalleryData &galleryData, int websiteID)
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
			reportError("subWebsite querey error", output, ("website = " + galleryData.subWebsiteName + "\npath = " + galleryData.path), false);
			return -1;
		}
		vector <vector<string>> modelsInDB; 
		dbDataParser.parseDBOutput(output, 2, modelsInDB);
		return atoi(modelsInDB[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> dbSubWebInfo;
	dbSubWebInfo.push_back(make_pair("path", path));
	dbSubWebInfo.push_back(make_pair("name", galleryData.subWebsiteName));
	dbSubWebInfo.push_back(make_pair("ParentID", to_string(websiteID)));
	dbCtrlr.insertNewDataEntry("SubWebsite", dbSubWebInfo, output);

	if (!output.empty())
	{
		reportError("subWebsite input error", output, ("website = " + galleryData.subWebsiteName + "\npath = " + galleryData.path), false);
		return -1;
	}
	return getLatestID();
}
//---------------------------------------------------------------------------------------------------------
int DatabaseBuilder::insertGalleryInfoIntoDB(GalleryData &galleryData, int websiteID, int subWebsiteID)
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
			reportError("gallery querey error", output, ("gallery = " + galleryData.galleryName + "\npath = " + galleryData.path), false);
			return -1;
		}
		vector <vector<string>> modelsInDB; 
		dbDataParser.parseDBOutput(output, 2, modelsInDB);
		return atoi(modelsInDB[0][0].c_str());
	}

	vector<DatabaseController::dbDataPair> dbGalleryInfo;
	dbGalleryInfo.push_back(make_pair("path", galleryData.path));
	dbGalleryInfo.push_back(make_pair("galleryName", galleryData.galleryName));
	dbGalleryInfo.push_back(make_pair("numModels", to_string(galleryData.models.size())));
	dbGalleryInfo.push_back(make_pair("WebsiteID", to_string(websiteID)));
	dbGalleryInfo.push_back(make_pair("SubWebsiteID", to_string(subWebsiteID)));
	dbCtrlr.insertNewDataEntry("Gallery", dbGalleryInfo, output);
	if (!output.empty())
	{
		reportError("gallery input error", output, ("gallery = " + galleryData.galleryName + "\npath = " + galleryData.path), false);
		return -1;
	}
	return getLatestID();
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::insertModelInfoIntoDB(GalleryModel &model)
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
			reportError("model querey error", output, ("model = " + model.name.firstName + " " + model.name.middleName + " " + model.name.lastName), false);
			return false;
		}
		vector <vector<string>> modelsInDB; 
		dbDataParser.parseDBOutput(output, 3, modelsInDB);
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
		reportError("model input error", output, ("model = " + model.name.firstName + " " + model.name.middleName + " " + model.name.lastName), false);
		return false;
	}

	model.name.dbID = getLatestID();
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::insertModelOutfitInfoIntoDB(GalleryModel &model, int clothingIndex, int galleryID)
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
		reportError("model outfit input error", output, ("model = " + model.name.firstName + " " + model.name.middleName + " " + model.name.lastName + "\ngalleryID = " + to_string(galleryID)), false);
		return false;
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::insertImageHashInfoIntoDB(string imgeFileName, string hash, string phash, string md5, int galleryID)
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
			reportError("image hash input error", output, "filename = " + imgeFileName + "\ngalleryID = " + to_string(galleryID), false);
			return false;
		}
	}
	return true;
}
//---------------------------------------------------------------------------------------------------------
bool DatabaseBuilder::insertModelsInGalleryInfoIntoDB(int modelID, int galleryID)
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
bool DatabaseBuilder::isImageInDB(int galleryID, string md5Hash)
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
			addEntryToInvalidPathFile(errString);
		}
		return true;
	}
	return false;
}