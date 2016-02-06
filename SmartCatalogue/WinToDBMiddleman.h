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
	void deleteImage(string image);
	void moveImage(string dest, string src);
	void deleteGallery(string gallery);
	void moveGallery(string dest, string src);
	
};