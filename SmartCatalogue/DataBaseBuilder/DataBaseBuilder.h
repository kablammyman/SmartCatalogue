#pragma once
#include <string>
#include <iostream>
#include <vector>

#include "DatabaseDataParser.h"
#include "DatabaseController.h"

#include "myFileDirDll.h"



using namespace std;

class DatabaseBuilder
{
private:
	DatabaseDataParser dbDataParser;
	DatabaseController dbCtrlr;
	string rootPath;//the root where all images will be found (optional)
	int getLatestID();
public:
	DatabaseBuilder(string dbPath, string root);
	void fillPartsOfSpeechTable(vector<string> &dbTableValues);
	void fillMetaWords(vector<string> &meta);
	int addEntryToInvalidPathFile(string message);
	bool parsePath(string curDir, GalleryData galleryData, string err);
	void reportError(string erreorType, string errorMessage, string extraInfo, bool outputToLog);
	int inserCategoryInfoIntoDB(GalleryData &galleryData);
	int insertWebsiteInfoIntoDB(GalleryData &galleryData, int categoryID);
	int insertSubWebsiteInfoIntoDB(GalleryData &galleryData, int websiteID);
	int insertGalleryInfoIntoDB(GalleryData &galleryData, int websiteID, int subWebsiteID, int categoryID);
	bool insertModelInfoIntoDB(GalleryModel &model);
	bool insertModelOutfitInfoIntoDB(GalleryModel &model, int clothingIndex, int galleryID);
	bool insertImageHashInfoIntoDB(string imgeFileName, string hash, string phash, string md5, int galleryID);
	bool insertModelsInGalleryInfoIntoDB(int modelID, int galleryID);
	bool DatabaseBuilder::isImageInDB(int galleryID, string md5Hash);
};