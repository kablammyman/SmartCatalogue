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

#define CATEGORY 0
#define WEBSITE 1
#define SUBWEBSITE 2
#define GALLERY 3
#define MODEL 4
class DatabaseDataParser
{
	//dictionary is used to for spellchecking. I use a trie instead of a normal vector for fast lookup times
	//since the dictionary will be bigger than 10 words, lol
	Trie *dictionary;
	Trie *keywords;
	//descriptiveWords is for reverse lookups, so i can see what DB to find a word in, without having to search them all
	//first is the actual word, the second is the catogory
	//ex: descriptiveWords["green"] -> color
	//ex: descriptiveWords["pants"] -> colothingItem
	map<string, string> descriptiveWords;

	//a quick refrence to partsof speech each table represents. this is all just data copied from a DB table for faster lookup times
	map<string, int> tableNamePartOfSpeech;

	//all the db table info in memory, agian, for faster lookup times
	map<string, vector<string>*> DBTables;
	

	FileWalker *fileWalker;
	DatabaseController *dbCtrl;

	string nameMarker;
	string seaerchForGalleryDescriptor(vector<string> &tokens, Trie *treeType);
	int getPartfOfSpeech(string word);
	bool isClothingNoun(string noun);
	bool isConjunction(string word);

	bool hasDataFromTable(string data, map<int,string> dbValues);
	//string getModelName(string path);
	vector<string> splitModelName(string input);
	vector<ModelName> doNameLogic(string allNames);
	void addMetaWordsToData(GalleryData &data);
	
	vector <ClothingItem> getOutfitFromGalleryName(string galleryName);
	//grab wordss that are in a DB table. if the word is misspelled, it wont be found
	//maybe we can add spell check to this?
	string getVerfiedWordFromGalleryName(string galleryName, string dbTableName);
	int getEntryIDFromDBTable(string trable, string word);
	//int getNameMarkerIndexInTokens(vector<string> & tokens);
	//int isNameMarkerInTokens(vector<string> & tokens);
public:
	DatabaseDataParser();

	bool getDBTableValues(string tableName);
	bool fillPartOfSpeechTable(string pofTableName);
	void setDBController(DatabaseController *dbc);
	void getAllPaths(string path, vector<string> &dirsWithImages, int numThreads = 0);
	bool calcGalleryData(string input, string ignorePattern, GalleryData &gallery, string &error);
	void transformClothingNameAlias(string &phrase);
	void transformSexToyAlias(string &phrase);
	void fillTreeWords(vector<string> &meta);
	
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