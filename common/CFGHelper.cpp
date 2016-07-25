#include "CFGHelper.h"
#include "CFGUtils.h"


string CFGHelper::filePathBase;
string CFGHelper::dbPath;
string CFGHelper::pathToProcess;
string CFGHelper::ignorePattern;
string CFGHelper::CreateImageHashIP;
int CFGHelper::CreateImageHashPort;
int CFGHelper::DataBaseManagerPort;
string CFGHelper::DataBaseManagerIP;
vector<string> CFGHelper::meta;

std::string CFGHelper::SetProgramPath(std::string argv)
{
	//MAX_PATH???
	char full[260];
	if (!argv.empty())
	{
		std::string temp = _fullpath(full, argv.c_str(), 260);
		size_t found = temp.find_last_of("/\\");
		return temp.substr(0, found);
	}
	else
	{
		std::string temp = _fullpath(full, argv.c_str(), 260);
		return temp;
	}
}


bool CFGHelper::LoadCFGFile(string programBasePath)
{
	string cfgPath;
	if (programBasePath.empty())
		cfgPath = CFGHelper::filePathBase + "\\imageViewCfg.txt";
	else
		cfgPath = programBasePath + "\\imageViewCfg.txt";

	if (!CFGUtils::ReadCfgFile(cfgPath, '|'))
	{
		string errorMsg = "Error opening :";
		errorMsg += cfgPath;
		//cout << errorMsg << "\nno cfg text file" << endl;
		return false;
	}

	dbPath = CFGUtils::GetCfgStringValue("DBPath");
	pathToProcess = CFGUtils::GetCfgStringValue("mainWorkingPath");
	ignorePattern = CFGUtils::GetCfgStringValue("ignorePattern");
	meta = CFGUtils::GetCfgListValue("metaWords");
	CreateImageHashIP = CFGUtils::GetCfgStringValue("CreateImageHashIP");
	CreateImageHashPort = CFGUtils::GetCfgIntValue("CreateImageHashPort");
	DataBaseManagerIP = CFGUtils::GetCfgStringValue("DataBaseManagerIP");
	DataBaseManagerPort = CFGUtils::GetCfgIntValue("DataBaseManagerPort");
	return true;
}
//check to make sure everything loaded
bool CFGHelper::IsCFGComplete(string &err)
{
	if (dbPath == "")
	{
		err = "dbPath is missing";
		return false;
	}
	if (pathToProcess == "")
	{
		err = "pathToProcess is missing";
		return false;
	}
	if (ignorePattern == "")
	{
		err = "ignorePattern is missing";
		return false;
	}
	if (CreateImageHashIP == "")
	{
		err = "CreateImageHashIP is missing";
		return false;
	}
	if (DataBaseManagerIP == "")
	{
		err = "DataBaseManagerIP is missing";
		return false;
	}
	if (CreateImageHashPort < 1000)
	{
		err = "CreateImageHashPort is missing or is less than 1000";
		return false;
	}
	if (DataBaseManagerPort < 1000)
	{
		err = "DataBaseManagerPort is missing or is less than 1000";
		return false;
	}
	return true;
}