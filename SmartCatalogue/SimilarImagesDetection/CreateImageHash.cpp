#include <iostream>

#include "similarImage.h"


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. Here are the options:\n";
	cout << "-compare <imgpath1> <imgpath2> -> compare 2 images\n";
	cout << "-compToDB <imgPath> <dbPath> -> compare an image to a a DB of image hashes\n";
	cout << "-compToDir <imgPath> <imgDir> -> compare an image to a directory of images\n";
	cout << "-hash <imgpath> create and return the hash of an image\n";
	cout << "-phash <imgpath> create and return the phash of an image\n";
	exit(1);
}

int main(int argc, const char *argv[])
{
	
	if (argc < 3)
		invalidParmMessageAndExit();

	int i = 0;
	
	string dbPath;
	string pathToProcess;

	while (i < argc)
	{
		if (strcmp(argv[i], "-dbPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			dbPath = argv[i];
		}
		else if (strcmp(argv[i], "-imgPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			pathToProcess = argv[i];
		}
		else if (strcmp(argv[i], "-hash") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			SimilarImage simImage;
			string returnVal = simImage.getImageHash(argv[i]);
			cout << returnVal;
			//cout << endl; //when other programs read from this programs stdout, we dont want the new line with the actual data
			
			return 0;
		}
		else if (strcmp(argv[i], "-phash") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			SimilarImage simImage;
			string returnVal = simImage.getImagePHash(argv[i]);
			cout << returnVal; 
		//	cout << endl;

			return 0;
		}
		else if (strcmp(argv[i], "-compare") == 0)// <imgpath1> <imgpath2> -> compare 2 images\n";
		{
			SimilarImage simImage;
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			string img1 = argv[i];

			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			string img2 = argv[i];

			cout << simImage.hashDistFrom2Images(img1,img2);
		}

		else if (strcmp(argv[i], "-compToDB") == 0)// <imgPath> <dbPath> -> compare an image to a a DB of image hashes\n";
		{
		}
		else if (strcmp(argv[i], "-compToDir") == 0)//  <imgPath> <imgDir> -> compare an image to a directory of images\n";
		{
		}

		i++;
	}

	return 0;
}