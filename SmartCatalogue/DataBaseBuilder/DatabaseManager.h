#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "MD5.h"

#include <Windows.h>

using namespace std;

bool doImageHash;



void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\\somePath\\\" -dataPath \"C:\\photos\\myFunPhotos\\\"\n";
	exit(-1);
}

string exec(const char* cmd)
{
	FILE* pipe;
	pipe = _popen(cmd, "r");
	if (!pipe)
		return "ERROR";
	char buffer[128];
	string result = "";

	while (fgets(buffer, 128, pipe) != NULL)
	{
		//hack until i figure out how to get rid of this message
		//***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****
		if (strcmp(buffer, "***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****\n") == 0)
			continue;
		if (strcmp(buffer, "\n") == 0)
			continue;
		result += buffer;
	}
	_pclose(pipe);
	return result;
}
//to use with args, there needs to be a space like so:
//myCreateProcess("C:\\windows\\notepad.exe", " example.txt");
void myCreateProcess(string pathAndName, string args)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (!
		CreateProcess(
			TEXT(pathAndName.c_str()),
			(LPSTR)args.c_str(),
			NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE,
			NULL, NULL,
			&si,
			&pi
			)
		)
	{
		cout << "Unable to execute.";
	}
}


//http://stackoverflow.com/questions/1220046/how-to-get-the-md5-hash-of-a-file-in-c
string createMD5Hash(string fileName)
{
	//Start opening your file
	ifstream inputFile;
	inputFile.open(fileName, std::ios::binary | std::ios::in);

	//Find length of file
	inputFile.seekg(0, std::ios::end);
	long len = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);

	//read in the data from your file
	char * InFileData = new char[len];
	inputFile.read(InFileData, len);

	//Calculate MD5 hash
	string returnString = md5(InFileData, len);

	//Clean up
	delete[] InFileData;

	return returnString;
}

