#pragma once

#include <string>
#include <utility>      // std::pair
#include "database.h"

using namespace std;

class DatabaseController
{
	DataBase *db;
	string dbName;

public:
	typedef pair <string, string> dbDataPair;

	void openDatabase(string path);
	bool isDBOpen();
	bool createNewDataBase(string newDbName);
	bool executeSQL(string command);
	string getDBName();
	string getTable(string tableName);
	string getLastError();

	bool insertNewDataEntry(string table, dbDataPair data, string &output);
	bool insertNewDataEntry(string table, vector<dbDataPair> data, string &output);

	//test methods
	void createTable(string tableName, string fields);
	void testGetTable();
};