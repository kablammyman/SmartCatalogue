#pragma once

#include <string>
#include "DatabaseController.h"
#include "myfiledirdll.h"

using namespace std;

class WinToDBMiddleman
{
private:
	DatabaseController dbCtrlr;
	string getGalleryIDQuereyString(string path);
public:
	void moveImage(string dest, string src);
	void moveGallery(string dest, string src);

	void deleteImage(string image);
	void deleteImageFromDisk(string image);
	void deleteImageFromDB(string image, string &output);

	void deleteGallery(string gallery);
	void deleteGalleryFromDB(string gallery, string &output);
	void deleteGalleryFromDisk(string gallery);
};