#pragma once
#include <string>
#include <vector>

#include "DatabaseDataParser.h"
#include "DatabaseController.h"
#include "TCPUtils.h"
#include "myFileDirDll.h"

#include "LogRouter.h"

using namespace std;

class DatabaseBuilder
{
private:
	DatabaseDataParser dbDataParser;
	DatabaseController dbCtrlr;
	string rootPath;//the root where all images will be found (optional)
	int GetLatestID();
	bool verboseOutput;
	LogRouter *logRouter;
public:
	
	void SetLogRouter(LogRouter *lr)
	{
		logRouter = lr;
	}
	DatabaseBuilder(string dbPath, string root);
	void FillPartsOfSpeechTable(vector<string> &dbTableValues);
	void FillMetaWords(vector<string> &meta);

	bool ParsePath(string curDir, GalleryData galleryData, string err);
	int InsertCategoryInfoIntoDB(GalleryData &galleryData);
	int InsertWebsiteInfoIntoDB(GalleryData &galleryData, int categoryID);
	int InsertSubWebsiteInfoIntoDB(GalleryData &galleryData, int websiteID);
	int InsertGalleryInfoIntoDB(GalleryData &galleryData, int websiteID, int subWebsiteID, int categoryID);
	bool InsertModelInfoIntoDB(GalleryModel &model);
	bool InsertModelOutfitInfoIntoDB(GalleryModel &model, size_t clothingIndex, int galleryID);
	bool InsertImageHashInfoIntoDB(string imgeFileName, string hash, string phash, string md5, int galleryID);
	bool InsertImageHashInfoIntoDB(string imgeFileName, string md5, int galleryID);
	bool AddHashDataToDB(string imgeFileName, string hash, string phash);
	bool InsertModelsInGalleryInfoIntoDB(int modelID, int galleryID);
	bool IsImageInDB(int galleryID, string md5Hash);
	void VerifyDB(string root);
	bool VerifyImage(string imagePath, int &galleryID);
	bool AddDirToDB(string curDir, bool doImageHash);
	bool RequestImageHashesForDir(string galleryPath, int galleryID, TCPUtils *conn, int connIndex);
	bool RequestImageHash(string imgPath, int galleryID, TCPUtils *conn, int connIndex);
};