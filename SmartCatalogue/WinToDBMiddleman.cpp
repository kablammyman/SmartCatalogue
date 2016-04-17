#include "stdafx.h"
#include "Windows.h"
#include "WinToDBMiddleman.h"

DatabaseController *WinToDBMiddleman::dbCtrlr;

void WinToDBMiddleman::InitDBController(string path)
{
	dbCtrlr = new DatabaseController();
}
void WinToDBMiddleman::SetDBController(DatabaseController *db)
{
	dbCtrlr = db;
}

bool WinToDBMiddleman::DeleteImage(string image)
{
	bool ret = DeleteImageFromDisk(image);
	if (!ret)
		return false;
	string output;
	ret = DeleteImageFromDB(image, output);
	if (!ret)
		return false;
	return true;
}

bool WinToDBMiddleman::DeleteImageFromDisk(string image)
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

bool WinToDBMiddleman::DeleteImageFromDB(string image, string &output)
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
string WinToDBMiddleman::GetGalleryIDQuereyStringFromPath(string path)
{
	string querey;
	querey = "SELECT ID FROM Gallery WHERE path = \"";
	querey += path;
	//querey += "\\\";";
	querey += "\";";
	return querey;
}

int WinToDBMiddleman::GetGalleryIDFromPath(string path)
{
	string output;
	string querey = GetGalleryIDQuereyStringFromPath(path);
	dbCtrlr->executeSQL(querey, output);
	
	if(output.empty())
		return -1;
	dbCtrlr->removeTableNameFromOutput(output);
	//get rid of new line
	output.pop_back();
	int x = atoi(output.c_str());
	return x;
}

bool WinToDBMiddleman::MoveImageOnDB(string dest, string src)
{
	string fileName = MyFileDirDll::getFileNameFromPathString(dest);
	string  querey, output;
	//get the old file in DB
	querey = "SELECT MD5 FROM Images WHERE galleryID = (";
	querey += GetGalleryIDQuereyStringFromPath(src);
	querey += ") AND fileName = \"";
	querey += MyFileDirDll::getFileNameFromPathString(src);
	querey += "\";";
	dbCtrlr->executeSQL(querey, output);

	vector<string> oldPathResult;
	dbCtrlr->getAllValuesFromCol(output, "MD5", oldPathResult);


	querey = "UPDATE Images Set Images.galleryID = (";
	querey += GetGalleryIDQuereyStringFromPath(dest);
	querey += ") WHERE MD5 IS \"";
	querey += oldPathResult[0];
	querey += "\";"; //the DB has the back slashes in the path, so i need to include them here
	dbCtrlr->executeSQL(querey, output);
	//cout << output << endl;
	if (output.empty())
		return true;
	return false;
}

bool WinToDBMiddleman::MoveImageOnDisk(string dest, string src)
{
	if (!MyFileDirDll::doesPathExist(dest))
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
	return true;
}

bool WinToDBMiddleman::MoveImage(string dest, string src)
{
	if(!MoveImageOnDisk(dest,src))
		return false;

	if(!MoveImageOnDB(dest, src))
		return false;

	return true;
}


bool WinToDBMiddleman::DeleteGalleryFromDisk(string gallery)
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

bool WinToDBMiddleman::DeleteGalleryFromDB(string gallery, string &output)
{
	string querey;
	//since a delete always "succeeds" even if there was nothing to delete, lets do a select first

	querey = "SELECT path FROM Gallery where path = \"";
	querey += gallery;
	querey += "\";";

	dbCtrlr->executeSQL(querey, output);
	
	if (!output.empty())
	{
		querey = "DELETE FROM Gallery where path = \"";
		querey += gallery;
		querey += "\";";
		return true;
	}
	
	//try with the trailing slash if it didnt find without one
	output.clear();
	querey = "SELECT path FROM Gallery where path = \"";
	querey += gallery;
	querey += "\\\";";

	dbCtrlr->executeSQL(querey, output);

	if (!output.empty())
	{
		querey = "DELETE FROM Gallery where path = \"";
		querey += gallery;
		querey += "\\\";";
		return true;
	}
	output = "could not find in DB: " + gallery;
	return false;
}

bool WinToDBMiddleman::DeleteGallery(string gallery)
{
	if (!DeleteGalleryFromDisk(gallery))
		return false;

	string output;
	if (!DeleteGalleryFromDB(gallery, output))
		return false;

	return true;
}

bool WinToDBMiddleman::DeleteGalleryAndImagesFromDB(string gallery, string &output)
{
	string querey;

	querey = "DELETE FROM Images  WHERE galleryID is (select id from Gallery where path = \"";
	querey += MyFileDirDll::getPathFromFullyQualifiedPathString(gallery);
	querey += "\\\");";

	dbCtrlr->executeSQL(querey, output);
	if(!output.empty())
		return false;

	return DeleteGalleryFromDB(gallery, output);
}
bool WinToDBMiddleman::MoveGalleryOnDisk(string dest, string src)
{
	if (!MoveFileExA(src.c_str(), dest.c_str(), MOVEFILE_COPY_ALLOWED))
	{
		string err = ("Error moving gallery: " + to_string(GetLastError()));
		//MessageBoxA(NULL, err.c_str(), NULL, NULL);
		return false;
	}
	return true;
}

bool WinToDBMiddleman::MoveGalleryOnDB(string dest, string src)
{
	string output, querey;

	//gotta see if this location exists already, if not, we have to create it
	string getDest = GetGalleryIDQuereyStringFromPath(dest);
	dbCtrlr->executeSQL(querey, output);

	//if the new gallery path doesnt exist, create it now!
	if (output.empty())
	{
		///do this eventually
	}

	querey = "UPDATE Images SET galleryID = ";
	querey += GetGalleryIDQuereyStringFromPath(dest);
	querey += " WHERE galleryID = ";
	querey += GetGalleryIDQuereyStringFromPath(src);
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

bool WinToDBMiddleman::MoveGallery(string dest, string src)
{
	if (!MoveGalleryOnDisk(dest, src))
		return false;

	if (!MoveGalleryOnDB(dest, src))
		return false;

	return true;
}