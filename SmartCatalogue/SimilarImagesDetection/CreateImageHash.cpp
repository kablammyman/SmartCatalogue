#include <iostream>

#include "similarImage.h"


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. Here are the options:\n";
	cout << "-compare <imgpath1> <imgpath2> -> compare 2 images\n";
	cout << "-addToDB <imgPath> <dbPath> -> add either a single image to a DB\n";
	cout << "-addToDB <imgDir> <dbPath> -> add a direcotry of images to a DB\n";
	cout << "-compToDB <imgPath> <dbPath> -> compare an image to a a DB of image hashes\n";
	cout << "-compToDir <imgPath> <imgDir> -> compare an image to a directory of images\n";
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

			dbPath = argv[i + 1];
		}
		else if (strcmp(argv[i], "-imgPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			pathToProcess = argv[i + 1];
		}
		i++;
	}

	return 0;
}