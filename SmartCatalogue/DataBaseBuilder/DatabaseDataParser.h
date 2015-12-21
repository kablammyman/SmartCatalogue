#pragma once

#include <string>
#include <vector>
#include <map>
#include "Trie.h"

#include "DatabaseController.h"
#include "DatabaseTableStructs.h"
#include "fileWalker.h"

using namespace std;

#define UNKNOWN 0
#define NOUN 1
#define ADJ  2
#define VERB 3

class DatabaseDataParser
{
	Trie* keyWords;
	//first is the actual word, the second is the catogory
	//ex: descriptiveWords["green"] -> color
	//ex: descriptiveWords["pants"] -> colothingItem
	map<string, string> descriptiveWords;
	map<string, int> tableNamePartOfSpeech;
	map<string, vector<string>*> DBTables;
	

	FileWalker *fileWalker;
	DatabaseController *dbCtrl;

	string nameMarker;
	string seaerchForGalleryDescriptor(vector<string> &tokens, Trie *treeType);
	int getPartfOfSpeech(string word);
	bool isClothingNoun(string noun);
	bool isConjunction(string word);

	bool hasDataFromTable(string data, map<int,string> dbValues);
	string getModelName(string path);
	vector<string> tokenize(string path, string delims);
	vector<string> splitModelName(string input);
	vector<ModelName> doNameLogic(string allNames);
	void addMetaWordsToData(string path, GalleryData &data);
	void toProperNoun(string &input);
	vector <ClothingItem> getOutfitFromGalleryName(string galleryName);
	int getEntryIDFromDBTable(string trable, string word);
	//int getNameMarkerIndexInTokens(vector<string> & tokens);
	//int isNameMarkerInTokens(vector<string> & tokens);
public:
	void getDBTableValues(string tableName);
	vector <vector<string>>parseDBOutput(string &inputData, int numFields);
	void fillPartOfSpeechTable(string pofTableName);
	DatabaseDataParser();
	void setDBController(DatabaseController *dbc);
	void getAllPaths(string path);
	bool calcGalleryData(string input, string ignorePattern, vector<GalleryData *> &gallery);
	
	
	void fillTreeWords(vector<string> &meta, Trie *trieType);
	
	/*
	bool addToField(string type) {
	if(type == "Pantytypes")
		doSomethingt...
	}
	*/
	
	void GetAllBadPaths();

	void testNamelogic();
	void testSpellchecker();
	void testGalleryCalc();
};