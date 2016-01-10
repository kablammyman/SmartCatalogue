#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>
#include "sqlite3.h"

class DataBase
{
	int returnCode;
	sqlite3 *db;
	std::string lastError;
	std::string databaseName;
	std::string curTableName;
	static int callback(void *data, int argc, char **argv, char **azColName);
	static std::string returnData;
	static bool gettingData;
public:
	DataBase(std::string name);
	DataBase();
	~DataBase();
	bool openDataBase(std::string name, std::string &output);
	void closeDataBase(std::string &output);
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
#endif //DATABASE_H