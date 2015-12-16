#include <iostream>

#include "similarImage.h"


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "CreateImageHash -dbPath \"C:\\somePath\\\" -imgPath \"C:\\photos\\myFunPhotos\\\"\n";
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