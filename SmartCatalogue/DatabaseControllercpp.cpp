#include "stdafx.h"
#include "DatabaseController.h"





void DatabaseController::openDatabase(string path)
{
	//prob should parse the path and seperate that from the name at some point
	//right nopw, path should only eb the name of the db file
	dbName = path;
	db = new DataBase(path);
	
	string output;

	if (!db->openDataBase(dbName, output))
	{
		printf("couldnt open %s, %s", path.c_str(), output.c_str());
		dbName = "";
		db->closeDataBase(output);
		return;
	}
	//curDBWindowData = db->viewData();
	//sendDataToEditWindow(getDBViewHandle(), curDBWindowData);
}

string DatabaseController::getDBName()
{
	return dbName;
}
bool DatabaseController::isDBOpen()
{
	if (db == NULL)
		return false;
	return true;
}

bool DatabaseController::executeSQL(string command)
{
	string output;
	return db->executeSQL(command, output);
	//curDBWindowData.clear();

	//sendDataToEditWindow(g_hMDIDBView, output);
	//SELECT * FROM images WHERE metaData ="stockings"
	//SELECT FirstName,modelLastName,path FROM images WHERE modelFirstName IS NOT NULL

}

bool DatabaseController::createNewDataBase(string newDBName)
{
	//curDBWindowData.clear();
	//close teh current one
	string output;
	db->closeDataBase(output);
	//curDBWindowData.push_back(output);

	FILE * pFile;
	pFile = fopen(newDBName.c_str(), "w");
	fclose(pFile);

	//create the new one
	db->openDataBase(newDBName, output);
	//curDBWindowData.push_back(output);

	string command = "CREATE TABLE 'galleries' ('path'	TEXT,'category'	TEXT,'websiteName' TEXT,'subWebsiteName' TEXT,'modelFirstName' TEXT,'modelLastName' TEXT,'galleryName' TEXT,'timesViewed'	INTEGER,'metaData' TEXT);";
	bool returnVal = db->executeSQL(command, output);
	//curDBWindowData.push_back(output);

	command = "CREATE TABLE 'models' ('firstName' TEXT, lastName' TEXT,'Alias' TEXT,'ethnicity'	TEXT,'faceImages' BLOB);";
	returnVal = db->executeSQL(command, output);
	//curDBWindowData.push_back(output);

	//sendDataToEditWindow(getDBViewHandle(), curDBWindowData);

	return returnVal;
}

string DatabaseController::getLastError()
{
	return db->getLastError();
}

string DatabaseController::getTable(string tableName)
{
	string returnString;
	string command = ("SELECT * FROM " + tableName);
	if (!db->executeSQL(command, returnString))
		return "";

	return returnString;
}

///////////////////////////////////////////////////////////test methods
void DatabaseController::createTable(string tableName, string fields)
{
	db->createTable(tableName, fields);
}

void DatabaseController::testGetTable()
{
	string result = getTable("BodyPart");
	printf("%s", result.c_str());

	/*DatabaseDataParser temp;
	temp.parseDBOutput(result, 2);*/
}

//returns true when successful
bool DatabaseController::insertNewDataEntry(string table, dbDataPair data, string &output)
{
	string querey = ("INSERT INTO " + table + " ("+data.first +")");
	string values = "VALUES ( \" + data.second +  \");";

	return db->executeSQL(querey + values, output);
	
}

//returns true when successful
bool DatabaseController::insertNewDataEntry(string table, vector<dbDataPair> data, string &output)
{
	string querey = ("INSERT INTO " + table + " (");
	string values = "VALUES ( ";
	for (size_t i = 0; i < data.size(); i++)
	{
		querey += data[i].first;
		values += "\"" + data[i].second + "\"";

		if (i < data.size() - 1)
		{
			querey += ",";
			values += ",";
		}
	}
	querey += ")";
	values += ");";

	return db->executeSQL(querey + values, output);

}