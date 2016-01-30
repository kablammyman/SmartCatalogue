#pragma once

#include <string>
#include <vector>

using namespace std;

class Utils
{
public:
	static string filePathBase;
	static string dbPath;
	static string pathToProcess;
	static string ignorePattern;
	static vector<string> meta;

	static string getExePath();
	static void setProgramPath(string argv);
	static void loadCFGFile(string programBasePath = "");
	static vector<string> tokenize(string path, string delims);
	static void toProperNoun(string &input);
};