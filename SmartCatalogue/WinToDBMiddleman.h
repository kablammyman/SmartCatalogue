#pragma once

#include <string>
#include "DatabaseController.h"
#include "myfiledirdll.h"

using namespace std;

class WinToDBMiddleman
{
private:
	static DatabaseController *dbCtrlr;
	static string getGalleryIDQuereyString(string path);
public:
	static void initDBController(string path);
	static void setDBController(DatabaseController *db);
	static bool moveImage(string dest, string src);
	static bool moveGallery(string dest, string src);

	static bool deleteImage(string image);
	static bool deleteImageFromDisk(string image);
	static bool deleteImageFromDB(string image, string &output);

	static bool deleteGalleryAndImagesFromDB(string gallery, string &output);

	static bool deleteGallery(string gallery);
	static bool deleteGalleryFromDB(string gallery, string &output);
	static bool deleteGalleryFromDisk(string gallery);
};