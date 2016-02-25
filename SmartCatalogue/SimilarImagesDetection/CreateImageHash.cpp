#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "myFileDirDll.h"
#include "similarImage.h"
#include "Database.h"


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. Here are the options:\n";
	cout << "-compare <imgpath1> <imgpath2> -> compare 2 images\n";
	cout << "-isInDB <imgPath> <dbPath> -> compare an image from disk or clipboard to a DB of image hashes\n";
	cout << "-isInDir <imgPath> <imgDir> -> compare an image to a directory of images\n";
	cout << "-hash <imgpath> create and return the hash of an image\n";
	cout << "-phash <imgpath> create and return the phash of an image\n";
	cout << "-dist <int> (optional) specify how close the images are to be considered a match\n";
	cout << std::flush;
	exit(1);
}

std::string getClipboardStringData()
{
	if (!OpenClipboard(NULL))
	{
		printf("Can't open clipboard");
		return "";
	}

	HANDLE textData = GetClipboardData(CF_TEXT);
	std::string rturnstring((char *)textData);

	CloseClipboard();
	return rturnstring;
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

Mat getClipboardImage()
{
	Mat returnMat;
	if (!OpenClipboard(NULL))
	{
		printf("Can't open clipboard");
		return returnMat;
	}

	if (IsClipboardFormatAvailable(CF_BITMAP) || IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_DIBV5))
	{
		HBITMAP hbmp = (HBITMAP)GetClipboardData(CF_BITMAP);

		if (hbmp != NULL && hbmp != INVALID_HANDLE_VALUE)
		{
			IplImage *img = hBitmapToIpl(hbmp);


			returnMat = cv::cvarrToMat(img);

			/*cvShowImage("image", img);
			cvWaitKey();
			cvDestroyWindow("image");*/
		}
	}

	CloseClipboard();

	return returnMat;
}
int main(int argc, const char *argv[])
{
	
	if (argc < 3)
		invalidParmMessageAndExit();

	int i = 0;
	SimilarImage simImage;
	string dbPath;
	string pathToProcess;
	string output = "";
	bool err = false;
	string cmdArgUpper;

	for (int j = 0; j < argc; j++)
	{
		string temp = argv[j];
		if (temp == "-dist")
		{
			simImage.setMinHammingDist(atoi(argv[j + 1]));
		}
	}
	while (i < argc)
	{
		cmdArgUpper = argv[i];
		for (size_t j = 0; j < cmdArgUpper.size(); j++)
			cmdArgUpper[j] = toupper(cmdArgUpper[j]);
			

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
		else if (cmdArgUpper == "-HASH")
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			
			output = simImage.getImageHash(argv[i]);
			break;
		}
		else if (cmdArgUpper == "-PHASH")
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			output = simImage.getImagePHash(argv[i]);
			break;
		}
		else if (cmdArgUpper == "-COMPARE")// <imgpath1> <imgpath2> -> compare 2 images\n";
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			string img1 = argv[i];

			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			string img2 = argv[i];

			int diff = simImage.hashDistFrom2Images(img1,img2);
			if (diff == -1)
				exit(-1);
			output = (to_string(simImage.getPercentDiff(diff)) +  "% simularity");
			break;
		}
		else if (cmdArgUpper == "-ISINDIR")//  <imgPath> <imgDir> -> compare an image to a directory of images\n";
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			string img1Hash = simImage.getImageHash(argv[i]);
			if (img1Hash.empty())
			{
				output = "invalid file: "; 
				output += argv[i];
				err = true;
			}
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			vector<string> files = MyFileDirDll::getAllFileNamesInDir(argv[i]);
			output = "possible matches:\n";
			for (size_t j = 0; j < files.size(); j++)
			{
				string hash = simImage.getImageHash(files[j]);
				if (simImage.HanmingDistance(img1Hash, hash) < simImage.getMinHammingDist())
					output += (files[j] + "\n");
			}
			//remove the last comma since its not needed
			output.resize(output.length() - 1);
			break;
		}

		else if (cmdArgUpper == "-ISINDB")// <imgPath> <dbPath> -> compare an image to a a DB of image hashes\n";
		{
			string img1Hash;
			string dbPath;
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			string temp = MyFileDirDll::getFileNameFromPathString(argv[i]);
			//i can also look at the argc count, but this seems better
			if (temp[temp.size()-3] == '.' && temp[temp.size() - 2] == 'd' && temp[temp.size() - 1] == 'b') 
			{
				//we have the db path as first arg, check for clipboard data
				dbPath = argv[i];
				img1Hash = simImage.getImageHash(getClipboardImage());
				if (img1Hash.empty())
				{
					output = "no image data in clipboard or in command line params";
					err = true;
					break;
				}
			}
			else
			{
				img1Hash = simImage.getImageHash(argv[i]);
				if (img1Hash.empty())
				{
					cout << "invalid file: " << argv[i] << std::flush;
					exit(-1);
				}
				i++;
				if (i >= argc)
					invalidParmMessageAndExit();
				dbPath = argv[i];
				
			}
			DataBase db(dbPath);

			string querey = "SELECT  Images.fileName, Gallery.path FROM Images INNER JOIN Gallery ON Gallery.ID = Images.galleryID where hammingDistance('";
			querey += img1Hash;
			querey += "',hash) < ";
			querey += to_string(simImage.getMinHammingDist());
			querey += ";";
			db.executeSQL(querey, output);
			if (output.empty())
				output = "no matches found";
			break;
		}
		

		i++;
	}
	if (output.empty())
		output = "didnt recognize any params...";
	
	cout << output << std::flush;
	//cout << endl; //when other programs read from this programs stdout, we dont want the new line with the actual data
	if(!err)
		return 0;
	return -1;
}