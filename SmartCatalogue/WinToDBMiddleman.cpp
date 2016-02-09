#include "stdafx.h"
#include "Windows.h"
#include "WinToDBMiddleman.h"

void WinToDBMiddleman::initDBController(string path)
{
	dbCtrlr = new DatabaseController();
}
void WinToDBMiddleman::setDBController(DatabaseController *db)
{
	dbCtrlr = db;
}

bool WinToDBMiddleman::deleteImage(string image)
{
	bool ret = deleteImageFromDisk(image);
	if (!ret)
		return false;
	string output;
	ret = deleteImageFromDB(image, output);
	if (!ret)
		return false;
	return true;
}

bool WinToDBMiddleman::deleteImageFromDisk(string image)
{
	if (image.size() < 3)// I guess the min length is 4... C:\a can be a legit file
		return false;

	if (!MyFileDirDll::deleteFile(image))
	{
		string err = ("Error deleting " + image);
	//	MessageBoxA(NULL, err.c_str(), NULL, NULL);
		return false;
	}
	return true;
}

bool WinToDBMiddleman::deleteImageFromDB(string image, string &output)
{
	string querey;
	querey = "DELETE FROM Images WHERE  Images.fileName = \"";
	querey += MyFileDirDll::getFileNameFromPathString(image);
	querey += "\" AND galleryID IS (SELECT galleryID FROM Gallery WHERE path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(image);
	querey += "\\\");"; //the DB has the back slashes in the path, so i need to include them here

	dbCtrlr->executeSQL(querey, output);
	//cout << output << endl;
	if (output.empty())
		return true;
	return false;
}
string WinToDBMiddleman::getGalleryIDQuereyString(string path)
{
	string querey;
	querey = "SELECT ID FROM Gallery WHERE path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(path);
	querey += "\\\";";
	return querey;
}

bool WinToDBMiddleman::moveImage(string dest, string src)
{
	string fileName = MyFileDirDll::getFileNameFromPathString(dest);


	if(!MyFileDirDll::doesPathExist(dest))
	{
		string err = ("Error, " + dest + " does not exist");
	//	MessageBoxA(NULL, err.c_str(), NULL, NULL);
		return false;
	}
	if (!MoveFileA(src.c_str(), dest.c_str()))
	{
		string err = ("Error moving file: " + to_string(GetLastError()));
	//	MessageBoxA(NULL, err.c_str(), NULL, NULL);
		return false;
	}


	string  querey, output;
	//get the old file in DB
	querey = "SELECT MD5 FROM Images WHERE galleryID = (";
	querey += getGalleryIDQuereyString(src);
	querey += ") AND fileName = \"";
	querey += MyFileDirDll::getFileNameFromPathString(src);
	querey += "\";";
	dbCtrlr->executeSQL(querey, output);

	vector<string> oldPathResult;
	dbCtrlr->getAllValuesFromCol(output, "MD5", oldPathResult);


	querey = "UPDATE Images Set Images.galleryID = (";
	querey += getGalleryIDQuereyString(dest);
	querey += ") WHERE MD5 IS \"";
	querey += oldPathResult[0];
	querey += "\";"; //the DB has the back slashes in the path, so i need to include them here
	dbCtrlr->executeSQL(querey, output);
	//cout << output << endl;
	if (output.empty())
		return true;
	return false;
}

bool WinToDBMiddleman::deleteGallery(string gallery)
{
	bool ret = deleteGalleryFromDisk(gallery);
	if (!ret)
		return false;
	string output;
	ret = deleteGalleryFromDB(gallery,output);
	if (!ret)
		return false;
	return true;
}

bool WinToDBMiddleman::deleteGalleryFromDisk(string gallery)
{
	string gallDir = MyFileDirDll::getPathFromFullyQualifiedPathString(gallery);
	string result = MyFileDirDll::deleteAllFilesInDir(gallDir);
	RemoveDirectoryA(gallDir.c_str());

	if (!result.empty())
	{
		//MessageBoxA(NULL, result.c_str(), NULL, NULL);
		return false;
	}
	return true;
}

bool WinToDBMiddleman::deleteGalleryFromDB(string gallery, string &output)
{
	string querey;

	querey = "DELETE FROM Gallery where path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(gallery);
	querey += "\\\";";

	dbCtrlr->executeSQL(querey, output);
	if (output.empty())
		return true;
	return false;
}

bool WinToDBMiddleman::deleteGalleryAndImagesFromDB(string gallery, string &output)
{
	string querey;

	querey = "DELETE FROM Images  WHERE galleryID is (select id from Gallery where path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(gallery);
	querey += "\\\");";

	dbCtrlr->executeSQL(querey, output);

	querey = "DELETE FROM Gallery where path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(gallery);
	querey += "\\\";";

	dbCtrlr->executeSQL(querey, output);
	if (output.empty())
		return true;
	return false;
}


bool WinToDBMiddleman::moveGallery(string dest, string src)
{
	
	if (!MoveFileExA(src.c_str(), dest.c_str(), MOVEFILE_COPY_ALLOWED ))
	{
		string err = ("Error moving gallery: " + to_string(GetLastError()));
		//MessageBoxA(NULL, err.c_str(), NULL, NULL);
		return false;
	}

	string output, querey;

	//gotta see if this location exists already, if not, we have to create it
	string getDest = getGalleryIDQuereyString(dest);
	dbCtrlr->executeSQL(querey, output);

	//if the new gallery path doesnt exist, create it now!
	if (output.empty())
	{
		///do this eventually
	}

	querey = "UPDATE Images SET galleryID = ";
	querey += getGalleryIDQuereyString(dest);
	querey += " WHERE galleryID = ";
	querey += getGalleryIDQuereyString(src);
	querey += ";";
	dbCtrlr->executeSQL(querey, output);

	querey = "delete FROM Gallery where path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(src);
	querey += "\\\";";

	dbCtrlr->executeSQL(querey, output);
	if (output.empty())
		return true;
	return false;
}

