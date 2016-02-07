#pragma once

#include <string>
#include <vector>

using namespace std;

class Utils
{
public:
	static string getExePath();
	static string setProgramPath(string argv);
	static vector<string> tokenize(string path, string delims);
	static void toProperNoun(string &input);
	static string getTimeStamp(double milis);
};