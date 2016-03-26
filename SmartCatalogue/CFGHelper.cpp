#include "stdafx.h"

#include "CFGHelper.h"
#include "CFGReaderDll.h"


string CFGHelper::filePathBase;
string CFGHelper::dbPath;
string CFGHelper::pathToProcess;
string CFGHelper::ignorePattern;
string CFGHelper::CreateImageHashIP;
int CFGHelper::CreateImageHashPort;
int CFGHelper::DataBaseManagerPort;
string CFGHelper::DataBaseManagerIP;
vector<string> CFGHelper::meta;

void CFGHelper::loadCFGFile(string programBasePath)
{
	string cfgPath;
	if (programBasePath.empty())
		cfgPath = CFGHelper::filePathBase + "\\imageViewCfg.txt";
	else
		cfgPath = programBasePath + "\\imageViewCfg.txt";

	if (!CFG::CFGReaderDLL::readCfgFile(cfgPath, '|'))
	{
		string errorMsg = "Error opening :";
		errorMsg += cfgPath;
		//cout << errorMsg << "\nno cfg text file" << endl;
	}

	dbPath = CFG::CFGReaderDLL::getCfgStringValue("DBPath");
	pathToProcess = CFG::CFGReaderDLL::getCfgStringValue("mainWorkingPath");
	ignorePattern = CFG::CFGReaderDLL::getCfgStringValue("ignorePattern");
	meta = CFG::CFGReaderDLL::getCfgListValue("metaWords");
	CreateImageHashIP = CFG::CFGReaderDLL::getCfgStringValue("CreateImageHashIP");
	CreateImageHashPort = CFG::CFGReaderDLL::getCfgIntValue("CreateImageHashPort");
	DataBaseManagerIP = CFG::CFGReaderDLL::getCfgStringValue("DataBaseManagerIP");
	DataBaseManagerPort = CFG::CFGReaderDLL::getCfgIntValue("DataBaseManagerPort");

}