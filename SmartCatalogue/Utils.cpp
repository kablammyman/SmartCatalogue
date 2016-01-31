#include "stdafx.h"

#include <iostream>
#include <Windows.h>
#include "CFGReaderDll.h"
#include "Utils.h"

string Utils::filePathBase;
string Utils::dbPath;
string Utils::pathToProcess;
string Utils::ignorePattern;
vector<string> Utils::meta;

string Utils::getExePath()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	string::size_type pos = string(buffer).find_last_of("\\/");
	if (pos == string::npos)
		return "";

	return string(buffer).substr(0, pos);
}

void Utils::setProgramPath(string argv)
{
	char full[MAX_PATH];
	string temp = _fullpath(full, argv.c_str(), MAX_PATH);
	size_t found = temp.find_last_of("/\\");
	filePathBase = temp.substr(0, found);
}

void Utils::loadCFGFile(string programBasePath)
{
	string cfgPath;
	if (programBasePath.empty())
		cfgPath = Utils::filePathBase + "\\imageViewCfg.txt";
	else
		cfgPath = programBasePath + "\\imageViewCfg.txt";

	if (!CFG::CFGReaderDLL::readCfgFile(cfgPath, '|'))
	{
		string errorMsg = "Error opening :";
		errorMsg += cfgPath;
		cout << errorMsg << "\nno cfg text file" << endl;
	}

	dbPath = CFG::CFGReaderDLL::getCfgStringValue("DBPath");
	pathToProcess = CFG::CFGReaderDLL::getCfgStringValue("mainWorkingPath");
	ignorePattern = CFG::CFGReaderDLL::getCfgStringValue("ignorePattern");
	meta = CFG::CFGReaderDLL::getCfgListValue("metaWords");
}

vector<string> Utils::tokenize(string path, string delims)
{
	vector<string> returnVec;
	char *p = strtok(const_cast<char *>(path.c_str()), delims.c_str());
	while (p)
	{
		//printf ("Token: %s\n", p);
		returnVec.push_back(p);
		p = strtok(NULL, delims.c_str());
	}
	return returnVec;
}

//turns words or sentences into all proper nouns
void Utils::toProperNoun(string &input)
{
	bool caps = true;
	for (size_t i = 0; i < input.size(); i++)
	{

		if (input[i] == ' ' || input[i] == '-' || input[i] == '_')
		{
			caps = true;
			continue;
		}
		if (caps)
		{
			input[i] = toupper(input[i]);
			caps = false;
		}
		else
			input[i] = tolower(input[i]);
	}
}

string Utils::getTimeStamp(double milis)
{
	int seconds = milis / 1000;
	int minutes = seconds / 60;
	seconds %= 60;
	int hours = minutes / 60;
	minutes %= 60;
	int days = hours / 24;
	hours %= 24;
	string output;

	if (days > 0)
		output += (to_string(days) + " days ");
	if (hours > 0)
		output += (to_string(hours) + " hours ");
	if (minutes > 0)
		output += (to_string(minutes) + " minutes ");
	if (seconds > 0)
		output += (to_string(seconds) + " seconds");

	return output;
}
