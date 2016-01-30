#pragma once

#include <string>
#include <utility>      // std::pair
#include "database.h"
#include <vector>

using namespace std;

class DatabaseController
{
	DataBase *db;
	string dbName;

public:
	typedef pair <string, string> dbDataPair;
	DatabaseController();
	~DatabaseController();

	bool openDatabase(string path);
	bool isDBOpen();
	bool createNewDataBase(string newDBName, string createCommand="");
	bool executeSQL(string command, string &output);
	string getDBName();
	string getTable(string tableName);
	string getLastError();

	bool insertNewDataEntry(string table, dbDataPair data, string &output);
	bool insertNewDataEntry(string table, vector<dbDataPair> data, string &output);
	bool doDBQuerey(string table, string data, string &output);
	bool doDBQuerey(string table, vector<string> data, string &output);
	bool doDBQuerey(string table, vector<dbDataPair> data, string &output);
	void parseDBOutput(string &inputData, int numFields, vector <vector<string>> &returnData);
	//num cols is hopw many cols were truthned from the querey, and the colToUse is the data we want to put in the vector
	void removeTableNameFromOutput(string &inputData, int numCols, int colToUse, vector<string> &returnData);
	//same as before, but now we can couple data together
	void removeTableNameFromOutput(string &inputData, int numCols, int colToUse1, int colToUse2,  vector<dbDataPair> &returnData);

	//test methods
	void createTable(string tableName, string fields);
	void testGetTable();
	void testDBEntry();
	void testDBQuerey();
};