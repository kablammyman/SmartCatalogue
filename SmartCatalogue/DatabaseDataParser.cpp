#include "stdafx.h"


#include "DatabaseDataParser.h"



#define TEST_DATABASE_PARSER 1

DatabaseDataParser::DatabaseDataParser()
{
	/*in the cfg file, theres a list of all the "drop down" db tables name
	we use that list to know what values are allowed...we dont want miss-spellings
	*/
	keyWords = new Trie();

	nameMarker = "models";//when we see this in a path, we know that the next token is a name for sure
	fileWalker = new FileWalker();

#ifdef TEST_DATABASE_PARSER
	//testNamelogic();
	//testSpellchecker();
	testGalleryCalc();
#endif

	//if (!meta.empty())
	//	fileWalker->fillMetaWords(meta);
}
//----------------------------------------------------------------------
void DatabaseDataParser::setDBController(DatabaseController *dbc)
{
	//create our own db controller if one doesnt exist that we can borrow
	if (dbc == NULL)
		dbCtrl = new DatabaseController();
	else
		dbCtrl = dbc;
}
//----------------------------------------------------------------------
vector<string> DatabaseDataParser::tokenize(string path, string delims)
{
	vector<string> returnVec;
	char *p = strtok(const_cast<char *>(path.c_str()), delims.c_str());
	while (p)
	{
		//printf ("Token: %s\n", p);
		returnVec.push_back(p);
		p = strtok(NULL, delims.c_str());
	}
	return returnVec;
}

//we have quite a few tables that act like drop down..we will need theri values for parsing data
void DatabaseDataParser::getDBTableValues(string tableName)
{
	int numCol = 2;
	string result = dbCtrl->getTable(tableName);

	if (result == "")
		return;

	vector<vector<string>> table = parseDBOutput(result, numCol);
	
	if (table.size() < (size_t)numCol)
		return;

	if (!DBTables[tableName])
	{
		vector<string> *newData = new vector<string>();
		for (size_t i = 0; i < table[1].size(); i++)
			newData->push_back(table[1][i]);

		DBTables[tableName] = newData;
	}
}

//----------------------------------------------------------------------
//turns words or sentences into all proper nouns
void DatabaseDataParser::toProperNoun(string &input)
{
	bool caps = true;
	for (size_t i = 0; i < input.size(); i++)
	{

		if (input[i] == ' ' || input[i] == '-' || input[i] == '_')
		{
			caps = true;
			continue;
		}
		if (caps)
		{
			input[i] = toupper(input[i]);
			caps = false;
		}
		else
			input[i] = tolower(input[i]);
	}
}
//----------------------------------------------------------------------
//after we trhow away opltiplex//porno pics//some folder//
//we should be left with: porn stars//name or glam sites//website//name
//so, if we get rid of words that have "site" and other tell tale words that are not names, we narrow down the list
//',' '&', or "and" often mean 2 or more names are included
//if theres a space, then we may have a name
//if there are no numbers (or just 1 digit) we may have a name


//this will return a llist of names, each entry is a full name, but now delimianted b commas
vector<string> DatabaseDataParser::splitModelName(string input)
{
	vector<string> names;
	size_t foundSymbol;

	foundSymbol = input.find('&');
	if (foundSymbol != string::npos)
		input.replace(foundSymbol, 1, ",");

	foundSymbol = input.find(" and ");
	if (foundSymbol != string::npos)
		input.replace(foundSymbol, 5, ",");

	names = tokenize(input, ",");


	if (names.size() == 0)//we onyl have 1 name
		names.push_back(input);

	return names;
}
//----------------------------------------------------------------------
//if the name contains a '-' or '_' that means the name has 2 in one. ex Jenna Von_Oy -> Von_Oy is 2 names for a last name
vector<ModelName> DatabaseDataParser::doNameLogic(string allNames)
{
	vector<string> modelNames = splitModelName(allNames);
	vector<ModelName> nameVec;

	//even thouh if theres 2 names, this wont work right, in the porn stars and am dirs, that cant happen, so we are safe
	for (size_t i = 0; i < modelNames.size(); i++)
	{
		ModelName model;
		vector<string> names = tokenize(modelNames[i], " ");
		if (!names.empty())
		{
			names[0][0] = toupper(names[0][0]);//capatolize the first letter of each name
			model.firstName = names[0];
		}
		if (names.size() > 1)
		{
			bool useMiddleName = names.size() > 2 ? true : false;
			for (size_t j = 1; j < names.size(); j++)
			{
				names[j][0] = toupper(names[j][0]);//capatolize the first letter of each name
				if (useMiddleName && j == 1)
					model.middleName = names[j];
				else //we may have a 4 named person...? if so this should cover that
					model.lastName += (names[j]+ " ");
			}
		}
		
		//remove trailing white space i added
		if(model.lastName != "")
			model.lastName.resize(model.lastName.find_last_of(" "));

		if (!model.firstName.empty())
			nameVec.push_back(model);
	}
	return nameVec;
}

bool DatabaseDataParser::calcGalleryData(string input, string ignorePattern, vector<GalleryData *> &gallery)
{
	/*
	use a state machine tofigure thigns out.
	we will go thru the tokens, and with each token, transition to a new state
	biggest one is once we see a "models" token, transition to getName state. from there, we know we wont see a subwebsite for example. only more models or a gallery

	*/
	string path = input;
	string catogory, website, subwebsite, galleryName;
	size_t pos = input.find(ignorePattern);
	input = input.substr(pos + ignorePattern.size());

	vector<string> otherModels; //when we have a gallery with more than 1 model in the porn star or amature model catogory
	vector<string> tokens = tokenize(input, "\\");

	//minimun is catogory/website/gallery
	if (tokens.size() < 3)
		return false;

	int curState = 0;
	vector<ModelName> names;

	//sometimes we may get an empty value at the end from the trailing slash
	if (tokens[tokens.size() - 1] == "\r")//the is the carrage return
		tokens.pop_back();

	for (size_t i = 0; i < tokens.size(); i++)
	{

		// sometiomes we have more than 1 model folder, so find them all
		//ex: porn stars\sunny leone\models\Aria Giovanni\poolside play (porn stars and models indicate names)
		if (tokens[i] == "models" || tokens[i] == "porn stars" || tokens[i] == "amatuer models")
		{
			curState = 4;
			continue;
		}


		switch (curState)
		{
		case 0:
			catogory = tokens[i];
			if (i < tokens.size() && tokens[i + 1] != "models")
				curState = 1;
			break;
		case 1:
			toProperNoun(tokens[i]);
			website = tokens[i];
			//if we have more than 1 token left, and ther next one isnt models
			if (((tokens.size() - 1) - i) > 1 && tokens[i + 1] != "models")
				curState = 2;
			else if (((tokens.size() - 1) - i) <= 1 && tokens[i + 1] != "models")
				curState = 3;

			break;
		case 2:
			toProperNoun(tokens[i]);
			subwebsite = tokens[i];
			if (i < tokens.size() && tokens[i + 1] != "models")
				curState = 3;
			break;
		case 3:
			toProperNoun(tokens[i]);
			galleryName = tokens[i];
			if (i < (tokens.size() - 1))
				return false;
			break;
		case 4:
			//the acutal model's name is after the model index
			vector<ModelName> temp = doNameLogic(tokens[i]);
			//sometimes we get more names in a porn star dir..ie, shes witha  friend
			//so combine all the names into 1 vector
			names.insert(names.end(), temp.begin(), temp.end());
			curState = 3;
			break;
		}
	}

	if (galleryName.empty())
		return false;

	//now, make as many gallery data structs as there are names
	size_t index = 0;
	do {
		GalleryData *galleryData = new GalleryData();
		galleryData->path = path;

		galleryData->category = catogory;
		galleryData->websiteName = website;
		galleryData->subWebsiteName = subwebsite;
		galleryData->galleryName = galleryName;

		//if we dont have a name, skip this
		if (index < names.size())
		{
			GalleryModel tempModel;
			tempModel.name = names[index];
			//ClothingItem item;

			tempModel.outfit = getOutfitFromGalleryName(galleryName);
			galleryData->models.push_back(tempModel);
		}
		addMetaWordsToData(path, (*galleryData));

		gallery.push_back(galleryData);
		index++;
	} while (index < names.size());

	return true;

}
//----------------------------------------------------------------------
void DatabaseDataParser::fillTreeWords(vector<string> &meta, Trie *trieType)
{
	for (size_t i = 0; i < meta.size(); i++)
		trieType->addWord(meta[i]);
}
//----------------------------------------------------------------------
string DatabaseDataParser::seaerchForGalleryDescriptor(vector<string> &tokens, Trie *treeType)
{
	for (size_t i = 0; i < tokens.size(); i++)
		if (treeType->searchWord(tokens[i]))
			return tokens[i];
	return "";
}
//----------------------------------------------------------------------
//find all the intersting words this path has. Later, add a random img file name and see if the filename has some meta keywords
void DatabaseDataParser::addMetaWordsToData(string path, GalleryData &data)
{
	string metaWords = "";
	vector<string> tokens = tokenize(path, "- \\");
	for (size_t i = 0; i < tokens.size(); i++)
	{
		//need a smart way to do this. i have to get the adjs for each nouun
		//ex:
		//purple skirt blue room. the purple is for the clothing item, blue describes location
	}

}
vector <ClothingItem> DatabaseDataParser::getOutfitFromGalleryName(string galleryName)
{
	vector <ClothingItem> clothes;

	return clothes;
}

//----------------------------------------------------------------------
bool DatabaseDataParser::hasDataFromTable(string data, map<int, string> dbValues)
{
	return false;
}
//----------------------------------------------------------------------
void DatabaseDataParser::getAllPaths(string path)
{
	fileWalker->takeDirSnapShot(path);
}

//----------------------------------------------------------------------
vector<vector<string>> DatabaseDataParser::parseDBOutput(string &inputData,int numFields)
{
	vector <vector<string>> returnData(numFields, vector<string>(0));//initialise tahe num vectors we need with 1 elements each
	int curField = 0;
	vector<string> tokens = tokenize(inputData, "|\n");
	//the first is the field name, the second is the value we want
	for (size_t i = 1; i < tokens.size(); i+=2)
	{
		returnData[curField].push_back(tokens[i]);
		if (curField == numFields-1)
			curField = 0;
		else
			curField++;
			
	}

	return returnData;
}


/////////////////////////////are these still needed or useful?////////////////////////////////
/*size_t DatabaseDataParser::getNameMarkerIndexInTokens(vector<string> & tokens)
{
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (tokens[i] == nameMarker)
			return i;
	}
	return -1;
}*/

/////////////////////////////////////////////////TESTS///////////////////////////////////////////
void DatabaseDataParser::testNamelogic()
{
	string nameTestString = "first middle last, first2 last2, first3, first4 middle4 midle4-2 last4";
	doNameLogic(nameTestString);
}



void DatabaseDataParser::testGalleryCalc()
{
	vector<string> testPaths;
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\539");//no name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\models\\anita e\\a-lonely-dreamer");//one model
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\models\\eveline and nancay\\outdoor play in panties");//two models
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\penthouse\\models\\Austin, Alyssa Reece, Yurizan Beltran and Melanie Jayne\\waterfall");//4 models
	
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\59561");//sub website, no name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\models\\anita pearl\\black lace bustier black pattern pantyhose");//sub website, 1 model
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\models\\Kristina Blond, Jennifer Stone and Thalia Festiny");//sub website, 3 models

	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls"); //bad format...not enough info
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\539\\fakeDir"); //bad format...no model name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\models\\anita e\\a-lonely\\fake dir");//bad format, too many dirs after model name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\models\\Kristina Blond, Jennifer Stone and Thalia Festiny\\something fake");//sub website, 3 models


	for (size_t i = 0; i < testPaths.size(); i++)
	{
		vector<GalleryData *> gallery;
		bool ret = calcGalleryData(testPaths[i], "\\\\OPTIPLEX-745\\photos\\porno pics", gallery);
		if (ret)
		{
			printf("its good!");
			for (size_t j = 0; j < gallery.size(); j++)
			{
				printf("%s\n", gallery[j]->path.c_str());
				for (size_t k = 0; k < gallery[j]->models.size(); k++)
				{
					printf("%s\n", gallery[j]->models[k].name.firstName.c_str());
					printf("%s\n", gallery[j]->models[k].name.middleName.c_str());
					printf("%s\n", gallery[j]->models[k].name.firstName.c_str());
					if(true)
					printf("%d\n", gallery[j]->models[k].outfit[0].clotingTypeIndex);
					printf("%d\n", gallery[j]->models[k].sexActionIndex);
					printf("%d\n", gallery[j]->models[k].hairColorIndex);
				}
				printf("%s\n", gallery[j]->websiteName.c_str());
				printf("%s\n", gallery[j]->category.c_str());
				printf("%s\n", gallery[j]->subWebsiteName.c_str());
				printf("%s\n", gallery[j]->galleryName.c_str());
				//printf("%s\n", gallery[j]->metaData.c_str());
			}
		}
		else
			printf("its bad!");
	}
}
//-------------------------------------------------------------------------------------
//this shows paths that dont follow the proper format
void DatabaseDataParser::GetAllBadPaths()
{
	//fileWalker->getAllDirsWithBadPath(ignorePattern,curFileWindowData);
	/*
	fileWalker->getAllDirsWithImgs(curFileWindowData, false);

	for (size_t i = curFileWindowData.size() - 1; i > 0; i--)
	{
		//remove any paths that are legal, and only show the bad ones
		vector<GalleryData *> data;
		if (fileWalker->calcGalleryData(curFileWindowData[i], ignorePattern, data))
			curFileWindowData.erase(curFileWindowData.begin() + i);
	}
	//get last one
	vector<GalleryData *> data;
	if (fileWalker->calcGalleryData(curFileWindowData[0], ignorePattern, data))
		curFileWindowData.erase(curFileWindowData.begin());

	//sendDataToEditWindow(getFileViewHandle(), curFileWindowData);
	*/
}

/*
void DatabaseDataParser::getAllDirsWithBadPath(string ignorePattern, vector<string> & badDirs)
{
	
	//dont in FileWalker
	vector<string> allDirs = FileDir::MyFileDirDll::dumpTreeToVector(true);
	for (size_t i = 0; i < allDirs.size(); i++)
	{
		string input = allDirs[i];
		if (!ignorePattern.empty())
		{
			size_t pos = input.find(ignorePattern);
			input = input.substr(pos + ignorePattern.size());
		}
		vector<string> tokens = tokenize(input, "\\");
		if (tokens.size() > 5)
		{
			badDirs.push_back(allDirs[i]);
		}
	}
}*/