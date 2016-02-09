#pragma once

#include <string>
#include "DatabaseController.h"
#include "myfiledirdll.h"

using namespace std;

class WinToDBMiddleman
{
private:
	DatabaseController *dbCtrlr;
	string getGalleryIDQuereyString(string path);
public:
	void initDBController(string path);
	void setDBController(DatabaseController *db);
	bool moveImage(string dest, string src);
	bool moveGallery(string dest, string src);

	bool deleteImage(string image);
	bool deleteImageFromDisk(string image);
	bool deleteImageFromDB(string image, string &output);

	bool deleteGalleryAndImagesFromDB(string gallery, string &output);

	bool deleteGallery(string gallery);
	bool deleteGalleryFromDB(string gallery, string &output);
	bool deleteGalleryFromDisk(string gallery);
};