#include "stdafx.h"
#include "database.h"
#include <ios>
#include <iostream>
#include <string>

 
using namespace std;
string DataBase::returnData;
bool DataBase::gettingData;

static void testFunc(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	// check that we have four arguments (lat1, lon1, lat2, lon2)
/*	if (argc != 4)
		return;*/
	// check that all four arguments are non-null
	if (sqlite3_value_type(argv[0]) == SQLITE_NULL || sqlite3_value_type(argv[1]) == SQLITE_NULL /*|| sqlite3_value_type(argv[2]) == SQLITE_NULL || sqlite3_value_type(argv[3]) == SQLITE_NULL*/) {
		sqlite3_result_null(context);
		return;
	}
	// get the four argument values
	/*double lat1 = sqlite3_value_double(argv[0]);
	double lon1 = sqlite3_value_double(argv[1]);
	double lat2 = sqlite3_value_double(argv[2]);
	double lon2 = sqlite3_value_double(argv[3]);*/
	char *string1 = (char *)sqlite3_value_text(argv[0]);
	char *string2 = (char *)sqlite3_value_text(argv[1]);
	string ret = string1;
	ret += "~~~";
	ret += string2;
	//cout <<"mine: " << ret << endl;
	char *buffer = new char[ret.size()];
	strcpy(buffer, ret.c_str());
	sqlite3_result_text(context, buffer, strlen(buffer),NULL);
}
static void testFunc2(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	char *string1 = (char *)sqlite3_value_text(argv[0]);
	char *string2 = (char *)sqlite3_value_text(argv[1]);
	string ret = string1;
	ret += "~~~";
	ret += string2;

	char *buffer = new char[ret.size()];
	strcpy(buffer, ret.c_str());
	//trying to see what i can do to limit what i return, so i picked to only return "small strings"
	if(ret.size() < 9)
		sqlite3_result_text(context, buffer, strlen(buffer), NULL);

	//it doesnt matter if this is here or not for this method
	//else
	//	sqlite3_result_null(context);

	//success!!
	//dbCtrlr.executeSQL("SELECT testFunc(id,name) FROM BodyPart where id = 1;", output);
	//success!! it will only return the data that has less than 9 chars...looks like: testFunc2(id,name) | 1~~~Head
	//dbCtrlr.executeSQL("SELECT testFunc2(id,name) FROM BodyPart where testFunc2(id,name) NOT NULL;", output);
	//can i only reutnr special cols now? YES
	//dbCtrlr.executeSQL("SELECT name FROM Gallery where testFunc2(id,name) NOT NULL;", output);
	//what happens with a large table? seems super fast!
	//dbCtrlr.executeSQL("SELECT galleryName FROM Gallery where testFunc2(id,galleryName) NOT NULL;", output);

	//lets add in our own data instead of using query data for params
	//dbCtrlr.executeSQL("SELECT galleryName FROM Gallery where testFunc2('yup',id) NOT NULL;", output);
}

static void hammingDistance(sqlite3_context *context, int argc, sqlite3_value **argv)
{
	char *string1 = (char *)sqlite3_value_text(argv[0]);
	char *string2 = (char *)sqlite3_value_text(argv[1]);

	int difference = 0;
	for (int i = 0; i<64; i++)
	{
		if (string1[i] != string2[i])
			difference++;
	}
	
	sqlite3_result_int(context, difference);
}

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
	//a custom sqlite method test..needs to go right after i open the db
	int numParams = 2;//this var is more for remnding me what things are for
	//sqlite3_create_function(db, "testFunc", numParams, SQLITE_UTF8, NULL, &testFunc, NULL, NULL);
	//sqlite3_create_function(db, "testFunc2", numParams, SQLITE_UTF8, NULL, &testFunc2, NULL, NULL);
	sqlite3_create_function(db, "hammingDistance", 2, SQLITE_UTF8, NULL, &hammingDistance, NULL, NULL);
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
