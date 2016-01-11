#pragma once
#include <string>
#include <iostream>

#include "DatabaseDataParser.h"
#include "DatabaseController.h"

#include "CFGReaderDll.h"
#include "myFileDirDll.h"

using namespace std;

string filePathBase;
string cfgPath;
bool doImageHash;

DatabaseDataParser dbDataParser;
DatabaseController dbCtrlr;

string dbPath;
string pathToProcess;
string ignorePattern;//we dont need the base path for all processing, since it wont change


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\\somePath\\\" -dataPath \"C:\\photos\\myFunPhotos\\\"\n";
	exit(-1);
}

int getLatestID()
{
	string output;
	//get the id that was just created from the insert
	dbCtrlr.executeSQL("SELECT last_insert_rowid()", output);
	//othe ouput looks like: last_insert_rowid()|6

	size_t found = output.find_last_of("|");
	string rowID = output.substr(found + 1);
	return atoi(rowID.c_str());
}


string exec(const char* cmd) 
{
	FILE* pipe; 
	pipe = _popen(cmd, "r"); 
	if (!pipe) 
		return "ERROR";
	char buffer[128];
	string result = "";
	
	while (fgets(buffer, 128, pipe) != NULL)
	{
		//hack until i figure out how to get rid of this message
		//***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****
		if (strcmp(buffer, "***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****\n") == 0)
			continue;
		if (strcmp(buffer, "\n") == 0)
			continue;
		result += buffer;
	}
	_pclose(pipe);
	return result;
}

int addEntryToInvalidPathFile(string file)
{
	FILE * pFile;
	pFile = fopen("invalidPaths.txt", "a");
	if (pFile == NULL)
		return -1;

	fputs(file.c_str(), pFile);
	fclose(pFile);
	
	return 0;
}

void printTimeStamp(double milis)
{
	int seconds = milis / 1000;
	int minutes = seconds / 60;
	seconds %= 60;
	int hours = minutes / 60;
	minutes %= 60;
	int days = hours / 24;
	hours %= 24;
	cout << days << " days " << hours << " hours " << minutes << " minutes " << seconds << " seconds\n";
}

int insertWebsiteInfoIntoDB(GalleryData &galleryData)
{
	string output;
	vector<DatabaseController::dbDataPair> dbWebInfo;

	string path = (ignorePattern + galleryData.category + "\\" + galleryData.websiteName);
	dbWebInfo.push_back(make_pair("path", path));
	dbWebInfo.push_back(make_pair("name", galleryData.websiteName));
	dbCtrlr.insertNewDataEntry("Website", dbWebInfo, output);
	if (!output.empty())
	{
		cout << "website input error: " << output << endl;
		return -1;
	}
	return getLatestID();
}

int insertSubWebsiteInfoIntoDB(GalleryData &galleryData, int websiteID)
{
	string output;
	vector<DatabaseController::dbDataPair> dbSubWebInfo;

	string path = (ignorePattern + galleryData.category + "\\" + galleryData.websiteName + "\\" + galleryData.subWebsiteName);
	dbSubWebInfo.push_back(make_pair("path", path));
	dbSubWebInfo.push_back(make_pair("name", galleryData.subWebsiteName));
	dbSubWebInfo.push_back(make_pair("ParentID", to_string(websiteID)));
	if (!output.empty())
	{
		cout << "subWebsite input error: " << output << endl;
		return -1;
	}
	return getLatestID();
}

int insertGalleryInfoIntoDB(GalleryData &galleryData, int websiteID, int subWebsiteID)
{
	string output;
	vector<DatabaseController::dbDataPair> dbGalleryInfo;
	dbGalleryInfo.push_back(make_pair("path", galleryData.path));
	dbGalleryInfo.push_back(make_pair("galleryName", galleryData.galleryName));
	dbGalleryInfo.push_back(make_pair("numModels", to_string(galleryData.models.size())));
	dbGalleryInfo.push_back(make_pair("WebsiteID", to_string(websiteID)));
	dbGalleryInfo.push_back(make_pair("SubWebsiteID", to_string(subWebsiteID)));
	dbCtrlr.insertNewDataEntry("Gallery", dbGalleryInfo, output);
	if (!output.empty())
	{
		cout << "gallery input error: " << output << endl;
		return -1;
	}
	return getLatestID();
}

bool insertModelInfoIntoDB(GalleryModel &model)
{
	string output;
	vector<DatabaseController::dbDataPair> dbModelInfo;
	dbModelInfo.push_back(make_pair("firstName", model.name.firstName));
	dbModelInfo.push_back(make_pair("middleName", model.name.middleName));
	dbModelInfo.push_back(make_pair("lastName", model.name.lastName));
	dbCtrlr.insertNewDataEntry("Models", dbModelInfo, output);

	if (!output.empty())
	{
		cout << "model input error: " << output << endl;
		return false;
	}

	model.name.dbID = getLatestID();
	return true;
}

bool insertModelOutfitInfoIntoDB(GalleryModel &model, int clothingIndex, int galleryID)
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
		cout << "model outfit input error: " << output << endl;
		return false;
	}
	return true;
}

bool insertImageHashInfoIntoDB(string imgeFilePath, string hash, string phash, int galleryID)
{
	string output;
	vector<DatabaseController::dbDataPair> dbImgInfo;
	dbImgInfo.push_back(make_pair("fileName", imgeFilePath));
	dbImgInfo.push_back(make_pair("hash", hash));
	dbImgInfo.push_back(make_pair("phash", phash));
	dbImgInfo.push_back(make_pair("galleryID", to_string(galleryID)));
	dbCtrlr.insertNewDataEntry("Images", dbImgInfo, output);

	if (!output.empty())
	{
		//if we are trying to input the same image again (lie we restarted the process)
		//then its not an error
		if (output.find("UNIQUE constraint failed") == string::npos)
		{
			cout << "image hash input error: " << output << endl;
			return false;
		}
	}
	return true;
}