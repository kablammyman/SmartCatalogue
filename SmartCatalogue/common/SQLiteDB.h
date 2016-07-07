#ifndef SQLiteDB_H
#define SQLiteDB_H

#include <string>
#include <vector>
#include "sqlite3.h"

class SQLiteDB
{
	int returnCode;
	sqlite3 *db;
	std::string lastError;
	std::string SQLiteDBName;
	std::string curTableName;
	static int callback(void *data, int argc, char **argv, char **azColName);
	static std::string returnData;
	static bool gettingData;
public:
	SQLiteDB(std::string name);
	SQLiteDB();
	~SQLiteDB();
	bool openSQLiteDB(std::string name, std::string &output);
	void closeSQLiteDB(std::string &output);
	std::string getLastError();
	bool executeSQL(std::string command, std::string &output);

	bool createTable(std::string name, std::string query);
	std::vector<std::string> displayTable();
	
	std::vector<std::string> viewData() ;
	std::vector<std::string> query(char* query);
	std::vector<std::vector<std::string>> queryV2(char* query);
	std::vector<std::string> getAllTables();
	bool insertData(std::string query); 
	void setTableName(std::string name);
};
#endif //SQLiteDB_H