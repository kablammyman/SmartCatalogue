#pragma once

#include <string>
#include <vector>
#include <map>
#include "Trie.h"

#include "DatabaseController.h"
#include "DatabaseTableStructs.h"
#include "fileWalker.h"

using namespace std;

class DatabaseDataParser
{
	Trie* keyWords;

	map<string, vector<string>*> DBTables;
	/*map<int, string> actionPart;
	map<int, string> colors;
	map<int, string> material;
	map<int, string> print;
	map<int, string> toy;
	map<int, string> lingerie;
	map<int, string> clothing;
	map<int, string> scenery;*/

	FileWalker *fileWalker;
	DatabaseController *dbCtrl;

	string nameMarker;
	string seaerchForGalleryDescriptor(vector<string> &tokens, Trie *treeType);
	
	

	bool hasDataFromTable(string data, map<int,string> dbValues);
	string getModelName(string path);
	vector<string> tokenize(string path, string delims);
	vector<string> splitModelName(string input);
	vector<ModelName> doNameLogic(string allNames);
	void addMetaWordsToData(string path, GalleryData &data);
	void toProperNoun(string &input);
	vector <ClothingItem> getOutfitFromGalleryName(string galleryName);

	//int getNameMarkerIndexInTokens(vector<string> & tokens);
	//int isNameMarkerInTokens(vector<string> & tokens);
public:
	void getDBTableValues(string tableName);
	vector <vector<string>>parseDBOutput(string &inputData, int numFields);

	DatabaseDataParser();
	void setDBController(DatabaseController *dbc);
	void getAllPaths(string path);
	bool calcGalleryData(string input, string ignorePattern, vector<GalleryData *> &gallery);
	
	
	void fillTreeWords(vector<string> &meta, Trie *trieType);
	
	
	
	void GetAllBadPaths();

	void testNamelogic();
	void testSpellchecker();
	void testGalleryCalc();
};