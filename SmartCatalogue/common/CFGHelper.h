#pragma once

#include <string>
#include <vector>

using namespace std;

//when we need to use a cfg file, we always have these vars and shit, so i made it easy for each project to use the same code
class CFGHelper
{
public:
	static string filePathBase;
	static string dbPath;
	static string pathToProcess;
	static string ignorePattern;
	static vector<string> meta;
	static string CreateImageHashIP;
	static int CreateImageHashPort;
	static int DataBaseManagerPort;
	static string DataBaseManagerIP;
	
	static void loadCFGFile(string programBasePath = "");

	static bool IsCFGComplete(string &err);
};