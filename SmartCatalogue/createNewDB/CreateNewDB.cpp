// CreateNewDB.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm> 

#include "myFileDirDll.h"

using namespace std;

void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. Here are the options:\n";
	cout << "-dbData <path of db static data>\n";
	cout << "-dbPath <path of where new db will be placed>\n";

	exit(1);
}

int main(int argc, const char *argv[])
{

	if (argc < 3)
		invalidParmMessageAndExit();

	int i = 0;

	string dbDataDir;
	string dbPath;

	while (i < argc)
	{
		if (strcmp(argv[i], "-dbData") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			dbDataDir = argv[i];
		}
		else if (strcmp(argv[i], "-dbPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			dbPath = argv[i];
		}
		
		i++;
	}

	vector<string> allDBTextFiles = MyFileDirDll::getAllFileNamesInDir(dbDataDir);

	for (size_t i = 0; i < allDBTextFiles.size(); i++)
	{
		string curFile = allDBTextFiles[i];
		size_t found = curFile.find_last_of(".");
		
		if(curFile.substr(found + 1) != "txt")
			continue;

		string line;
		ifstream myfile(curFile);
		

		if (!myfile.is_open())
			cout << "Unable to open file " << curFile << endl;

		int lineCounter = 0;
		while (getline(myfile, line))
		{
			//the first line is the "create table" line
			if (lineCounter == 0)
			{

			}
			//all the other lines should go "X","someValue" , where X is the id value
			else
			{
				//first get the id and value
				found = line.find('"');//first quote
				found = line.find('"', found);//the second quote + 1

				string id = line.substr(0, found);
				string value = line.substr(found +1);

				//next we need to strip away the quotes
				id.erase(remove(id.begin(), id.end(), '"'), id.end());
				value.erase(remove(value.begin(), value.end(), '"'), value.end());
				
			}
			cout << line << '\n';
		}
		myfile.close();

	}

	return 0;
}
