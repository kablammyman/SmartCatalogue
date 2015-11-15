#pragma once

#include <string>

#include "database.h"

using namespace std;

class DatabaseController
{
	DataBase *db;
	string dbName;

public:
	void openDatabase(string path);
	bool isDBOpen();
	bool createNewDataBase();
	bool executeSQL(string command);
	string getDBName();
	string getTable(string tableName);
	string getLastError();
	//test methods
	void createTable(string tableName, string fields);
	void insertDataFromCurDir();
	void testGetTable();
};