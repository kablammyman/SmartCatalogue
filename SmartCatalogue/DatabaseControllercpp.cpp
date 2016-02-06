#include "stdafx.h"
#include "DatabaseController.h"
#include "Utils.h"

DatabaseController::DatabaseController()
{
	db = NULL;
	dbName = "";
}

DatabaseController::~DatabaseController()
{
	if (isDBOpen())
	{
		string output;
		db->closeDataBase(output);
	}
}

bool DatabaseController::openDatabase(string path)
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
		return false;
	}

	return true;
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

bool DatabaseController::executeSQL(string command, string &output)
{
	return db->executeSQL(command, output);
	//curDBWindowData.clear();

	//sendDataToEditWindow(g_hMDIDBView, output);
	//SELECT * FROM images WHERE metaData ="stockings"
	//SELECT FirstName,modelLastName,path FROM images WHERE modelFirstName IS NOT NULL

}

bool DatabaseController::createNewDataBase(string newDBName, string createCommand)
{
	//close the current db before making a new one
	string output;
	if (isDBOpen())
	{
		db->closeDataBase(output);
		delete db;
		db = NULL;
	}

	FILE * pFile;
	pFile = fopen(newDBName.c_str(), "w");
	fclose(pFile);

	//create the new db
	db = new DataBase(newDBName);

	//create the table, if we have a create command
	if(createCommand != "")
		return db->executeSQL(createCommand, output);

	return true;
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

bool DatabaseController::doDBQuerey(string table, string data, string &output)
{
	string querey = ("SELECT " + data + " FROM " + table + ";");
	return db->executeSQL(querey, output);
}

bool DatabaseController::doDBQuerey(string table, vector<string> data, string &output)
{
	string querey = "SELECT "; 

	for (size_t i = 0; i < data.size(); i++)
	{
		querey += data[i];
		if (i < data.size() - 1)
			querey += ",";
	}
	querey += (" FROM " + table + ";");
	return db->executeSQL(querey, output);
}

bool DatabaseController::doDBQuerey(string table, vector<dbDataPair> data, string &output)
{
	string querey = "SELECT ";
	string whereString = " WHERE ";
	string fromString = (" FROM " + table);

	//count how many specific items to querey
	//ex select * from table where name ="bla" and age = "bla"; \\2 whereArgs here
	int numWhereArgs = 1;

	for (size_t i = 0; i < data.size(); i++)
	{
		if (i > 0)
			querey += ",";
		

		querey += data[i].first;

		if (!data[i].second.empty())
		{
			if(numWhereArgs > 1)
				whereString += " AND ";
			whereString += (data[i].first +" = \"" + data[i].second + "\"");
			numWhereArgs++;
		}

		
	}
	whereString += ";";

	return db->executeSQL(querey+ fromString+ whereString, output);
}

//returns true when successful
bool DatabaseController::insertNewDataEntry(string table, dbDataPair data, string &output)
{
	string querey = ("INSERT INTO " + table + " (" + data.first + ")");
	string values = "VALUES ( \""+ data.second +  "\");";

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

void DatabaseController::parseDBOutput(string &inputData, int numFields, vector<vector<string>> &returnData)
{
	returnData.clear();
	returnData.resize(numFields, vector<string>(0));//initialise tahe num vectors we need with 1 elements each
	int curField = 0;
	vector<string> tokens = Utils::tokenize(inputData, "|\n");
	//the first is the field name, the second is the value we want
	for (size_t i = 1; i < tokens.size(); i += 2)
	{
		returnData[curField].push_back(tokens[i]);
		if (curField == numFields - 1)
			curField = 0;
		else
			curField++;

	}
}

string DatabaseController::dataGrabber(string &word, size_t &curPos)
{
	string curWord;
	curPos = word.find('|', curPos);
	curPos++;//move past the pipe and start to collect next word
	while (word[curPos] != '\n')
		curWord += word[curPos++];

	return curWord;
}

void DatabaseController::getAllValuesFromCol(string &inputData, string colName, vector<string> &returnData)
{
	bool done = false;
	string curWord;
	size_t c = 0;
	while (!done)
	{
		c = inputData.find(colName,c);
		if (c != string::npos)
			returnData.push_back(dataGrabber(inputData,c));
		
		else
			done = true;
	}
}



void DatabaseController::getDataPairFromOutput(string &inputData, string colName1, string colName2,vector<dbDataPair> &returnData)
{
	bool done = false;
	string firstWord;
	size_t c = 0;
	while (!done)
	{
		c = inputData.find(colName1,c);
		if (c != string::npos)
			firstWord = dataGrabber(inputData, c);

		c = inputData.find(colName2,c);
		if (c != string::npos)
			returnData.push_back(make_pair(firstWord, dataGrabber(inputData, c)));
		else
			done = true;

	}
}
//blah! the code is differnt enough that i cant reuse it woulth more loops... >=(
/*void DatabaseController::removeTableNameFromOutput(string &inputData, int numCols, int colToUse, vector<string> &returnData)
{
	if (colToUse > numCols || colToUse < 1)
		return;

	string delims = "|\n";
	int counter = 1;
	colToUse *= 2;


	char *p = strtok(const_cast<char *>(inputData.c_str()), delims.c_str());

	while (p)
	{
		if (counter == colToUse)
			returnData.push_back(p);
		//x2 becasue it goes colName|data, we only want the data, not the col name
		if (counter < (numCols * 2))
			counter++;
		else
			counter = 1;
		p = strtok(NULL, delims.c_str());
	}

}
*/

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

void DatabaseController::testDBEntry()
{
	string output;
	dbDataPair test1 = make_pair("name", "test");
	insertNewDataEntry("Category", test1, output);

	vector<dbDataPair> data;

	data.push_back(make_pair("ID", "000"));
	data.push_back(make_pair("WebsiteID", "001"));
	data.push_back(make_pair("SubWebsiteID", "002"));
	data.push_back(make_pair("path", "C:\\some\\fake\\path"));
	data.push_back(make_pair("tags", "test,testData,wow doge"));

	insertNewDataEntry("Gallery", data, output);


}


void DatabaseController::testDBQuerey()
{
	string output;
	 
	doDBQuerey("Category", "name", output);

	vector <vector<string>> returnData1; 
	parseDBOutput(output, 2, returnData1);

	vector<string> test2;
	test2.push_back("WebsiteID");
	test2.push_back("path");
	doDBQuerey("Gallery", test2, output);

	vector<dbDataPair> data;
	data.push_back(make_pair("ID", "000"));
	data.push_back(make_pair("WebsiteID", "001"));
	data.push_back(make_pair("SubWebsiteID", "002"));
	data.push_back(make_pair("path", "C:\\some\\fake\\path"));

	doDBQuerey("Gallery", data, output);

	
}
