#include "stdafx.h"
#include "DatabaseController.h"



//#define TEST_DATABASE_CONTROLLER 1

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

#ifdef TEST_DATABASE_CONTROLLER
	testGetTable();
#endif
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

bool DatabaseController::createNewDataBase()
{
	//curDBWindowData.clear();
	//close teh current one
	string output;
	db->closeDataBase(output);
	//curDBWindowData.push_back(output);

	string newDBName = dbName; //"pornoPics2.db";
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

void DatabaseController::insertDataFromCurDir()
{
	/*string querey, output, values;
	vector<string> data = getDataFromEditWindow(getFileViewHandle());

	if (data.empty())
		curDBWindowData.push_back("nothing to add to DB");

	for (size_t i = 0; i < data.size(); i++)
	{
		vector <GalleryData *>d;
		if (!fileWalker->calcGalleryData(data[i], ignorePattern, d))
		{
			string err = "invalid file path: " + data[i];
			curDBWindowData.push_back(err);
			continue;
		}

		for (size_t j = 0; j < d.size(); j++)
		{
			querey = "INSERT INTO galleries (path";
			values = "VALUES ( \"" + d[j]->path + "\"";

			if (!d[j]->model.firstName.empty())
			{
				querey += ",modelFirstName";
				values += ",\"" + d[j]->model.firstName + "\"";

				if (!d[j]->model.lastName.empty())
				{
					querey += ",modelLastName";
					values += ",\"" + d[j]->model.lastName + "\"";
				}
			}

			if (!d[j]->websiteName.empty())
			{
				querey += ",websiteName";
				values += ",\"" + d[j]->websiteName + "\"";
			}

			if (!d[j]->category.empty())
			{
				querey += ",category";
				values += ",\"" + d[j]->category + "\"";
			}

			if (!d[j]->subWebsiteName.empty())
			{
				querey += ",subWebsiteName";
				values += ",\"" + d[j]->subWebsiteName + "\"";
			}

			querey += ",galleryName";
			values += ",\"" + d[j]->galleryName + "\"";

			if (!d[j]->metaData.empty())
			{
				querey += ",metaData)";
				values += ",\"" + d[j]->metaData + "\");";
			}
			else
			{
				querey += ")";
				values += ");";
			}

			if (db->executeSQL(querey + values, output))
			{
				curDBWindowData.push_back(data[i]);
			}
		}
	}
	sendDataToEditWindow(getDBViewHandle(), curDBWindowData);*/
}