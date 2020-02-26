#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "FileUtils.h"
#include "WindowsUtils.h"
#include "ClipboardUtil.h"
#include "TCPUtils.h"
#include "CommandLineUtils.h"

#include "similarImage.h"
#include "SQLiteUtils.h"


#define DELIM "|"
TCPUtils conn;
SimilarImage simImage;

bool isServer = false;
int iResult = 0;
int numConn = 0;
int port = 2346;

HANDLE hNetworkThread;

using namespace std;


#define DEFAULT_BUFLEN 1024
#define NETWORK_ERROR -1
#define NETWORK_OK     0

struct Options
{
	int dist;
	string dbPath;
	string pathToProcess;
	
	Options()
	{
		dist = 2;
		dbPath = "";
		pathToProcess = "";
	}
};

struct HashCmdArg : CmdArg
{
	Options options;
	bool shutdown = false;
};

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

	vector<string> files = FileUtils::GetAllFileNamesInDir(dir);
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

// Hbitmap convert to IplImage
IplImage * hBitmapToIpl(HBITMAP hBmp)
{
	BITMAP bmp;
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	{
		return NULL;
	}
	// Get channels which equal 1 2 3 or 4 BmBitsPixel:
	// Specifies the number of bits Required to indicate the color of a pixel.
	int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8;

	// Get depth color bitmap or grayscale
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;


	// Create header image
	IplImage * img = cvCreateImage(cvSize(bmp.bmWidth, bmp.bmHeight), depth, nChannels);
	//IplImage * img = cvCreateImage(cvSize(533, 800), IPL_DEPTH_8U, 3);
	// Allocat memory for the pBuffer
	BYTE * pBuffer = new   BYTE[bmp.bmHeight * bmp.bmWidth * nChannels];
	//BYTE * pBuffer = new   BYTE[800 * 533 * 3];
	// Copies the bitmap bits of a specified device - dependent bitmap into a buffer
	GetBitmapBits(hBmp, bmp.bmHeight * bmp.bmWidth * nChannels, pBuffer);
	//GetBitmapBits(hBmp, 800 * 533 * 3, pBuffer);
	// Copy data to the imagedata
	memcpy(img->imageData, pBuffer, bmp.bmHeight * bmp.bmWidth * nChannels);
	//memcpy(img->imageData, pBuffer, 800 * 533 * 3);

	delete pBuffer;

	// Create the image
	IplImage * dst = cvCreateImage(cvGetSize(img), img->depth, 3);
	// Convert color
	cvCvtColor(img, dst, CV_BGRA2BGR);
	cvReleaseImage(&img);
	return dst;
}

//else if (cmdArgUpper == "-ISINDB")// <imgPath> <dbPath> -> compare an image to a a DB of image hashes\n";
string isClipboardImageInDB(string img, string dbPath)
{
	string img1Hash;
	string output;

	if(img.empty())
	{
		HBITMAP tempBitmap = ClipboardUtil::GetClipboardImageData();
		IplImage* ipl = hBitmapToIpl(tempBitmap);
		Mat imgData = cv::cvarrToMat(ipl);

		img1Hash = simImage.getImagePHash(imgData);
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
	SQLiteUtils db(dbPath);

	string querey = "SELECT  Images.fileName, Gallery.path FROM Images INNER JOIN Gallery ON Gallery.ID = Images.galleryID where hammingDistance('";
	querey += img1Hash;
	querey += "',phash) < ";
	querey += to_string(simImage.getMinHammingDist());
	querey += ";";
	db.DoDBQuerey(querey, output);
	if (output.empty())
		output = "no matches found";
	
	return output;
}

HashCmdArg parseCommand(vector<string> argv)
{
	int i = 0;
	HashCmdArg command;
	//std::vector<std::string> arguments(argv + 1, argv + argc);
	while (i < argv.size())
	{
		string cmdArgUpper = argv[i];

		for (size_t j = 0; j < cmdArgUpper.size(); j++)
			cmdArgUpper[j] = toupper(cmdArgUpper[j]);
		//spcial commands
		if (cmdArgUpper == "-SERVER")
		{
			isServer = true;
		}
		if (cmdArgUpper == "-EXIT")
		{
			command.shutdown = true;
		}
		else if (cmdArgUpper == "-DIST")
		{
			i++;
			if (i >= argv.size())
			{
				command.clear();
				return command;
			}
			command.options.dist = atoi(argv[i].c_str());
		}

		else if (cmdArgUpper == "-DBPATH")
		{
			i++;
			if (i >= argv.size())
			{
				command.clear();
				return command;
			};

			command.options.dbPath = argv[i];
		}

		else if (cmdArgUpper == "-IMGPATH")
		{
			i++;
			if (i >= argv.size())
			{
				command.clear();
				return command;
			}
			command.options.pathToProcess = argv[i];
		}

		else if (cmdArgUpper == "-HASH" || cmdArgUpper == "-PHASH" || cmdArgUpper == "-ALLHASH")
		{
			command.SetCommand(cmdArgUpper);
			i++;
			if (i >= argv.size())
			{
				command.clear();
				return command;
			}
			command.data.push_back(argv[i]);
		}
		else if (cmdArgUpper == "-COMPARE" || cmdArgUpper == "-ISINDIR")
		{
			int numArgs = 3; //-compare <img1> <img2>, or -isindir <imgPath> <imgDir>
			if (i+ numArgs >= argv.size())
			{
				command.clear();
				return command;
			}
			command.SetCommand(cmdArgUpper);
			i++;
			for (int a = i; a < numArgs; a++)
				command.data.push_back(argv[i]);
		}
		else if (cmdArgUpper == "-ISINDB")
		{
			command.SetCommand(cmdArgUpper);
			i++;
			//if we didnt supply an arg with this command, this signals to check the clipboard for an image
			if (i >= argv.size())
				command.data.push_back("");
		}

		i++;
	}
	//if we get here, we prob turned on/off server mode and thats it
	return command;
}

string  ExecuteCommand(HashCmdArg cmd)
{
	simImage.setMinHammingDist(cmd.options.dist);


	if(cmd.cmd.empty())
		return getCmdArgHelp();
	else if(cmd.cmd == "-HASH")
		return (cmd.data[0] + DELIM + simImage.getImageHash(cmd.data[0]));
	else if (cmd.cmd == "-PHASH")
		return (cmd.data[0] + DELIM + simImage.getImagePHash(cmd.data[0]));
	else if (cmd.cmd == "-ALLHASH")
		return (cmd.data[0] + DELIM + simImage.getAllHash(cmd.data[0]));
	else if (cmd.cmd == "-COMPARE")
		return compareTwoImages(cmd.data[0],cmd.data[1]);
	else if (cmd.cmd == "-ISINDIR")
		return getMatchesInDir(cmd.data[0], cmd.data[1]);
	else if (cmd.cmd == "-ISINDB")
		return isClipboardImageInDB(cmd.data[0], cmd.options.dbPath);

	return getCmdArgHelp();
}