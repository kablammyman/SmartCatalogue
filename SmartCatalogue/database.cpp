#include "stdafx.h"
#include "database.h"
#include <ios>
#include <iostream>
#include <string>

 
using namespace std;
string DataBase::returnData;
bool DataBase::gettingData;
DataBase::DataBase()
{
	returnCode = 0;
	db = NULL;
	lastError ="";
	databaseName="";
	curTableName="";
	returnData ="";
	gettingData = false;
}

DataBase::DataBase(string name)
{
	returnCode = 0;
	lastError = "";
	databaseName = "";
	curTableName = "";
	gettingData = false;
	openDataBase(name, returnData);
}
//--------------------------------------------------------------------------------------------------
DataBase::~DataBase()
{ 
	string output;
	closeDataBase(output);
}
//--------------------------------------------------------------------------------------------------
 bool DataBase::openDataBase(string name, string &output)
 {
	 // Open Database
	databaseName = name;
	
	if(name[name.size() -3] != '.' || name[name.size() -2] != 'd' || name[name.size() -1] != 'b')
		name +=".db";

	returnCode = sqlite3_open(name.c_str(), &db);
   //returnCode = sqlite3_open("MyDb.db", &db);
   if (returnCode)
   {
      string  errMsg = sqlite3_errmsg(db);
	  output += ("Error opening SQLite3 database: " +errMsg + "\n");
      sqlite3_close(db);
	  return false;
   }

   output += ("Opened " + name + "\n");
   lastError.clear();
   return true;
 }
//--------------------------------------------------------------------------------------------------
 void DataBase::closeDataBase(string &output)
 {
	  // Close Database if its open
	if (db == NULL)
		 return;
   output = ("Attepmting to Close " + databaseName + "...");
   sqlite3_close(db);
   output +=  "Closed";
   lastError.clear();
 }
//--------------------------------------------------------------------------------------------------
 string DataBase::getLastError()
 {
	 return lastError;
 }
//--------------------------------------------------------------------------------------------------
void DataBase::setTableName(std::string name)
{
	curTableName = name;
}
//--------------------------------------------------------------------------------------------------
int DataBase::callback(void *data, int argc, char **argv, char **azColName)
{
   //fprintf(stderr, "%s: ", (const char*)data);
	//cheap hack so we wait until all data has been collected
	gettingData = true;
	for(int i=0; i<argc; i++)
	{
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		string col = azColName[i];
		string val = "";

		if(argv[i] != NULL)
			val = argv[i];
			
		returnData += (col+ "|" + val+"\n");
	}
	gettingData = false;
	return 0;
}
//--------------------------------------------------------------------------------------------------
bool DataBase::executeSQL(string command, string &output)
{
	char *error;
	returnData = "";
	returnCode = sqlite3_exec(db, command.c_str(), callback, NULL, &error);
	if (returnCode)
	{
		//output = "Error executing SQLite3 statement: " + sqlite3_errmsg(db);
		output =  sqlite3_errmsg(db);
		lastError = error;
		sqlite3_free(error);
		return false;
	}
	while(gettingData){}//wait until return data is no longer empty
	output = returnData;
	lastError.clear();
	return true;
}
//--------------------------------------------------------------------------------------------------
bool DataBase::createTable(string name, string query)
{
	setTableName(name);
	string output;
	//string sqlCreateTable = "CREATE TABLE MyTable (id INTEGER PRIMARY KEY, value STRING);";
	string sqlCreateTable = "CREATE TABLE "+ curTableName + "(" +query+");";
	return executeSQL(sqlCreateTable,output);
}
//--------------------------------------------------------------------------------------------------
bool DataBase::insertData(string query) 
{
	string sqlInsert = "INSERT INTO " + curTableName +" VALUES(" + query + ");";
	string output;
	//string sqlInsert = "INSERT INTO MyTable VALUES(NULL, 'A Value');";
	return executeSQL(sqlInsert,output);
}
//--------------------------------------------------------------------------------------------------
vector<string> DataBase::getAllTables() 
{
	string getTables = "SELECT name FROM sqlite_master WHERE type='table'ORDER BY name;";
	//SELECT name FROM my_db.sqlite_master WHERE type='table';
	//executeSQL(getTables);
	//[x][y] x = tabel name [y] is...??
	
	return query(const_cast<char*>(getTables.c_str()));
}
//--------------------------------------------------------------------------------------------------
vector<string> DataBase::viewData() 
{
	string getTables = "SELECT value FROM galleries;";
	//SELECT name FROM my_db.sqlite_master WHERE type='table';
	//executeSQL(getTables);
	//so, it seems every colom in the tbale is returned as a vector
	//ex if my table has time, date, name, addy -> those 4 will be returned...this is one record
	//then each record is put into another vector
	return query(const_cast<char*>(getTables.c_str()));
}
//--------------------------------------------------------------------------------------------------
vector<string> DataBase::query(char* query)
{
	sqlite3_stmt *statement;
	vector<string>results;

	if(sqlite3_prepare_v2(db, query, -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		int result = 0;
		while(true)
		{
			result = sqlite3_step(statement);
			
			if(result == SQLITE_ROW)
			{
				string values;
				for(int col = 0; col < cols; col++)
				{
					values += ((char*)sqlite3_column_text(statement, col));
				}
				results.push_back(values);
			}
			else
			{
				break;   
			}
		}
	   
		sqlite3_finalize(statement);
	}
	
	string curError = sqlite3_errmsg(db);
	if(curError != "not an error") 
		lastError = curError;
	else
		lastError.clear();

	return results;  
}
//--------------------------------------------------------------------------------------------------
vector<vector<string>> DataBase::queryV2(char* query)
{
	sqlite3_stmt *statement;
	vector<vector<string> > results;

	if(sqlite3_prepare_v2(db, query, -1, &statement, 0) == SQLITE_OK)
	{
		int cols = sqlite3_column_count(statement);
		int result = 0;
		while(true)
		{
			result = sqlite3_step(statement);
			
			if(result == SQLITE_ROW)
			{
				vector<string> values;
				for(int col = 0; col < cols; col++)
				{
					values.push_back((char*)sqlite3_column_text(statement, col));
				}
				results.push_back(values);
			}
			else
			{
				break;   
			}
		}
	   
		sqlite3_finalize(statement);
	}
	
	string curError = sqlite3_errmsg(db);
	if(curError != "not an error") 
		lastError = curError;
	else
		lastError.clear();

	return results;  
}
