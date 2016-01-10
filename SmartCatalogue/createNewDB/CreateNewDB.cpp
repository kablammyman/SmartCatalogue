// CreateNewDB.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm> 

#include "myFileDirDll.h"
#include "DatabaseController.h"

using namespace std;

void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. Here are the options:\n";
	cout << "-dbData <path of db static data>\n";
	cout << "-dbPath <path of where new db will be placed>\n";

	exit(1);
}

int main(int argc, const char *argv[])
{

	if (argc < 3)
		invalidParmMessageAndExit();

	int i = 0;

	string dbDataDir;
	string dbPath;
	DatabaseController dbCtrlr;

	while (i < argc)
	{
		if (strcmp(argv[i], "-dbDataPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();

			dbDataDir = argv[i];
		}
		else if (strcmp(argv[i], "-dbPath") == 0)
		{
			i++;
			if (i >= argc)
				invalidParmMessageAndExit();
			dbPath = argv[i];
		}
		
		i++;
	}

	vector<string> allDBTextFiles = MyFileDirDll::getAllFileNamesInDir(dbDataDir);
	//create the new db
	dbCtrlr.createNewDataBase(dbPath);

	for (size_t i = 0; i < allDBTextFiles.size(); i++)
	{
		string curFile = allDBTextFiles[i];
		size_t found = curFile.find_last_of(".");
		
		if(curFile.substr(found + 1) != "txt")
			continue;

		string line;
		ifstream myfile(curFile);
		

		if (!myfile.is_open())
			cout << "Unable to open file " << curFile << endl;

		int lineCounter = 0;

		string tableName = "";
		vector<string> colNames; //most names will be the same, but 1 table isnt, so this should be able to handle that and other cases for the future
		string output;
		while (getline(myfile, line))
		{
			//the first line is the "create table" line, or for one file, every line creats a new table
			found = line.find("CREATE TABLE");
			if (lineCounter == 0 || found != string::npos)
			{
				tableName.clear();
				dbCtrlr.executeSQL(line,output);
				if (!output.empty())
					cout << "error: " << output << endl;
				//find the table name in the statement
				
				//by pass the space and the quote
				size_t i = (found + 12); 
				while (!isalpha(line[i]))
					i++;
				//once we are here, we should be in the table name
				while (isalpha(line[i]))
				{
					tableName.append(1, line[i]);
					i++;
				}

				if (!output.empty())
					cout << "error: " << output << endl;
				else
					cout << "created table: " << tableName << endl;

				string colCmd = ("PRAGMA table_info(" + tableName + ")");
				dbCtrlr.executeSQL(colCmd, output);

				found = output.find("name|");

				while (found != string::npos)
				{
					size_t newLine = output.find('\n',found);
					string newCol = output.substr(found + 5, newLine-(found + 5));
					colNames.push_back(newCol);
					found = output.find("name|", found + 5);
				}

				
			}
			//all the other lines should go "X","someValue" , where X is the id value
			else
			{
				string id;
				string value;

				//first, lets try to seperate tokens by a tab
				found = line.find('\t');
				if (found != string::npos)
				{
					id = line.substr(0, found);
					value = line.substr(found + 1);
				}
				else //seperate by the num of quotes
				{
					//first get the id and value
					found = line.find('"');//first quote
					found = line.find('"', found + 1);//the second quote + 1

					id = line.substr(0, found);
					value = line.substr(found + 1);
				}
				//next we need to strip away the quotes
				id.erase(remove(id.begin(), id.end(), '"'), id.end());
				value.erase(remove(value.begin(), value.end(), '"'), value.end());
				vector<DatabaseController::dbDataPair> entry;
				entry.push_back(make_pair(colNames[0], id));
				entry.push_back(make_pair(colNames[1], value));
				dbCtrlr.insertNewDataEntry(tableName,entry, output);
				if(!output.empty())
					cout << "error: " << output << endl;
			}
			lineCounter++;
			//cout << line << '\n';
		}
		myfile.close();

	}
	cout << "done!" << endl;
	return 0;
}
