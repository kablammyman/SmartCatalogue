#pragma once

#include <iostream>



#include "myFileDirDll.h"
#include "similarImage.h"
#include "Database.h"



#include "NetworkConnection.h"

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI doNetWorkCommunication(LPVOID lpParam);
DWORD WINAPI doDirWatch(LPVOID lpParam);

#define SERVICE_NAME  L"My Sample Service"

int iResult = 0;
int numConn = 0;
int port = 2346;
NetworkConnection conn;
HANDLE hNetworkThread;


using namespace std;


#define DEFAULT_BUFLEN 512
#define NETWORK_ERROR -1
#define NETWORK_OK     0

string getCmdArgHelp()
{
	string returnString = "invlaid parameters. Here are the options:\n";
	returnString += "-compare <imgpath1> <imgpath2> -> compare 2 images\n";
	returnString += "-isInDB <imgPath> <dbPath> -> compare an image from disk or clipboard to a DB of image hashes\n";
	returnString += "-isInDir <imgPath> <imgDir> -> compare an image to a directory of images\n";
	returnString += "-hash <imgpath> create and return the hash of an image\n";
	returnString += "-phash <imgpath> create and return the phash of an image\n";
	returnString += "-dist <int> (optional) specify how close the images are to be considered a match\n";

	return returnString;
}
string invalidParmMessageAndExit()
{
	cout << getCmdArgHelp <<endl;
	exit(-1);
}

//string img1 = argv[i];, string img2 = argv[i];
string compareTwoImages(string img1, string img2)
{
	int diff = simImage.hashDistFrom2Images(img1, img2);
	if (diff == -1)
		return "error with compare";
	return (to_string(simImage.getPercentDiff(diff)) + "% simularity");
}
//img = argv[i]
string getImageHash(stirng img)
{
	return simImage.getImageHash(img);
}
//img = argv[i]
string getImagePHash(stirng img)
{
	return simImage.getImagePHash(img);
}

//else if (cmdArgUpper == "-ISINDIR")//  <imgPath> <imgDir> -> compare an image to a directory of images\n";
string getMatchesInDir(string img, string dir)
{
	string img1Hash = simImage.getImagePHash(img);
	if (img1Hash.empty())
	{
		string output = "invalid file: ";
		output += img;
		return output;
		//err = true;
	}

	vector<string> files = MyFileDirDll::getAllFileNamesInDir(dir);
	string output = "possible matches:\n";
	for (size_t j = 0; j < files.size(); j++)
	{
		string hash = simImage.getImagePHash(files[j]);
		if (simImage.HanmingDistance(img1Hash, hash) < simImage.getMinHammingDist())
			output += (files[j] + "\n");
	}
	//remove the last comma since its not needed
	output.resize(output.length() - 1);
	return output;
}

//else if (cmdArgUpper == "-ISINDB")// <imgPath> <dbPath> -> compare an image to a a DB of image hashes\n";
string isClipboardImageInDB(string img, string dbPath)
{
	string img1Hash;
	string output;
		//i can also look at the argc count, but this seems better
	if(img.empty())
	{
		img1Hash = simImage.getImagePHash(getClipboardImage());
		if (img1Hash.empty())
		{
			string errorString = "no image data in clipboard or in command line params";
			return errorString;
			//err = true;
		}
	}
	else
	{
		img1Hash = simImage.getImagePHash(img);
		if (img1Hash.empty())
		{
			string errorString = "invalid file: ";
			errorString += img;
			return errorString;
			//err = true;
		}
	}
	DataBase db(dbPath);

	string querey = "SELECT  Images.fileName, Gallery.path FROM Images INNER JOIN Gallery ON Gallery.ID = Images.galleryID where hammingDistance('";
	querey += img1Hash;
	querey += "',phash) < ";
	querey += to_string(simImage.getMinHammingDist());
	querey += ";";
	db.executeSQL(querey, output);
	if (output.empty())
		output = "no matches found";
	
	return output;
}

vector<string> parseCommand(vector<string> argv)
{
	vector<string> commandsAndArgs;
	int i = 0;
	//std::vector<std::string> arguments(argv + 1, argv + argc);
	while (i < argv.size())
	{
		cmdArgUpper = argv[i];
		for (size_t j = 0; j < cmdArgUpper.size(); j++)
			cmdArgUpper[j] = toupper(cmdArgUpper[j]);

		if (temp == "-dist")
		{
			simImage.setMinHammingDist(atoi(argv[j + 1]));
		}

		if (cmdArgUpper == "-DBPATH")
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			dbPath = argv[i];
		}
		else if (cmdArgUpper == "-IMGPATH")
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			pathToProcess = argv[i];
		}

	

	


		i++;
	}
}