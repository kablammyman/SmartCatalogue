#pragma once

#include <string>
#include "DatabaseController.h"
#include "myfiledirdll.h"

using namespace std;

class WinToDBMiddleman
{
private:
	static DatabaseController *dbCtrlr;
	static string GetGalleryIDQuereyStringFromPath(string path);
public:
	static void InitDBController(string path);
	static void SetDBController(DatabaseController *db);

	static bool MoveImageOnDisk(string dest, string src);
	static bool MoveImageOnDB(string dest, string src);
	static bool MoveImage(string dest, string src);

	static bool MoveGalleryOnDB(string dest, string src);
	static bool MoveGalleryOnDisk(string dest, string src);
	static bool MoveGallery(string dest, string src);

	static bool DeleteImage(string image);
	static bool DeleteImageFromDisk(string image);
	static bool DeleteImageFromDB(string image, string &output);

	static bool DeleteGalleryAndImagesFromDB(string gallery, string &output);

	static bool DeleteGallery(string gallery);
	static bool DeleteGalleryFromDB(string gallery, string &output);
	static bool DeleteGalleryFromDisk(string gallery);

	static int GetGalleryIDFromPath(string path);
};