#include "stdafx.h"
#include "DatabaseDataParser.h"
#include "Utils.h"

DatabaseDataParser::DatabaseDataParser()
{
	/*in the cfg file, theres a list of all the "drop down" db tables name
	we use that list to know what values are allowed...we dont want miss-spellings
	*/
	dictionary = new Trie();
	keywords = new Trie();

	nameMarker = "models";//when we see this in a path, we know that the next token is a name for sure
	fileWalker = new FileWalker();
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

bool DatabaseDataParser::fillPartOfSpeechTable(string pofTableName)
{
	int numCol = 2;
	string result = dbCtrl->getTable(pofTableName);

	if (result == "")
		return false;

	vector<vector<string>> table = parseDBOutput(result, numCol);

	if (table.size() < (size_t)numCol)
		return false;

	for (size_t i = 0; i < table[1].size(); i++)
	{
		string tableName = table[0][i];
		string pos = table[1][i];
		Utils::toProperNoun(tableName);
		Utils::toProperNoun(pos);
		if (pos == "Noun")
			tableNamePartOfSpeech[tableName] = NOUN;
		else if (pos == "Adj")
			tableNamePartOfSpeech[tableName] = ADJ;
		else if (pos == "Verb")
			tableNamePartOfSpeech[tableName] = VERB;
		else
			tableNamePartOfSpeech[tableName] = UNKNOWN;
		
	}
	return true;
}
//we have quite a few tables that act like drop down..we will need theri values for parsing data
bool DatabaseDataParser::getDBTableValues(string tableName)
{
	int numCol = 2;
	string result = dbCtrl->getTable(tableName);

	if (result == "")
		return false;

	vector<vector<string>> table = parseDBOutput(result, numCol);
	
	if (table.size() < (size_t)numCol)
		return false;
	
	Utils::toProperNoun(tableName);
	if (!DBTables[tableName])
	{
		vector<string> *newData = new vector<string>();
		for (size_t i = 0; i < table[1].size(); i++)
		{ 
			string tableEntry = table[1][i];
			newData->push_back(tableEntry);
		//	if (tableName.empty() || tableEntry.empty())
		//		printf("wtf\n");
			descriptiveWords[tableEntry] = tableName;
			dictionary->addWord(tableEntry);
		}
		DBTables[tableName] = newData;
	}
	return true;
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

	names = Utils::tokenize(input, ",");


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
		vector<string> names = Utils::tokenize(modelNames[i], " ");
		if (!names.empty())
		{
			Utils::toProperNoun(names[0]);
			//names[0][0] = toupper(names[0][0]);//capatolize the first letter of each name
			model.firstName = names[0];
		}
		if (names.size() > 1)
		{
			bool useMiddleName = names.size() > 2 ? true : false;
			for (size_t j = 1; j < names.size(); j++)
			{
				Utils::toProperNoun(names[j]);
				//names[j][0] = toupper(names[j][0]);//capatolize the first letter of each name
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

bool DatabaseDataParser::calcGalleryData(string input, string ignorePattern, GalleryData &gallery, string &error)
{
	/*
	use a state machine to figure thigns out.
	we will go thru the tokens, and with each token, transition to a new state
	biggest one is once we see a "models" token, transition to getName state. from there, we know we wont see a subwebsite for example. only more models or a gallery

	*/
	string path = input;
	string catogory, website, subwebsite, galleryName;
	size_t pos = input.find(ignorePattern);
	input = input.substr(pos + ignorePattern.size());

	vector<string> otherModels; //when we have a gallery with more than 1 model in the porn star or amature model catogory
	vector<string> tokens = Utils::tokenize(input, "\\");

	//minimun is catogory/website/gallery
	if (tokens.size() < 3)
	{
		error = "path is illegeal, not enough descriptors aka, its too short!";
		return false;
	}
	int curState = 0;
	vector<ModelName> names;

	//sometimes we may get an empty value at the end from the trailing slash
	if (tokens[tokens.size() - 1] == "\r")//the is the carrage return
		tokens.pop_back();

	for (size_t i = 0; i < tokens.size(); i++)
	{

		// sometiomes we have more than 1 model folder, so find them all
		//ex: porn stars\sunny leone\models\Aria Giovanni\poolside play (porn stars and models indicate names)
		//note: we got rid of amatuer models and porn stars since they can be placed in model folders of actaul catogories
		if (tokens[i] == "models")
		{
			curState = MODEL;
			continue;
		}


		switch (curState)
		{
		case CATEGORY:
			catogory = tokens[i];
			if (i < tokens.size() && tokens[i + 1] != "models")
				curState = 1;
			break;
		case WEBSITE:
			Utils::toProperNoun(tokens[i]);
			website = tokens[i];
			//if we have more than 1 token left, and ther next one isnt models
			if (((tokens.size() - 1) - i) > 1 && tokens[i + 1] != "models")
				curState = 2;
			else if (((tokens.size() - 1) - i) <= 1 && tokens[i + 1] != "models")
				curState = 3;

			break;
		case SUBWEBSITE:
			Utils::toProperNoun(tokens[i]);
			subwebsite = tokens[i];
			if (i < tokens.size() && tokens[i + 1] != "models")
				curState = 3;
			break;
		case GALLERY:
			Utils::toProperNoun(tokens[i]);
			galleryName = tokens[i];
			if (i < (tokens.size() - 1))
			{
				error = "path is illegeal, gallery is at wrong level";
				return false;
			}
			break;
		case MODEL:
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
	{
		error = "path is illegeal, there is no gallery name";
		return false;
	}
	
	
	gallery.path = path;

	gallery.category = catogory;
	gallery.websiteName = website;
	gallery.subWebsiteName = subwebsite;
	gallery.galleryName = galleryName;
	gallery.location = getVerfiedWordFromGalleryName(galleryName, "Locations");
	gallery.toy = getVerfiedWordFromGalleryName(galleryName, "Sextoys");
	
	//now, make as many gallery data structs as there are names
	size_t index = 0;
	while (index < names.size()) 
	{
		//if we dont have a name, skip this

		GalleryModel tempModel;
		tempModel.name = names[index];
		tempModel.outfit = getOutfitFromGalleryName(galleryName);
		gallery.models.push_back(tempModel);
			
		index++;
	} 

	addMetaWordsToData(gallery);
	
	return true;
}
//----------------------------------------------------------------------
void DatabaseDataParser::fillTreeWords(vector<string> &meta)
{
	for (size_t i = 0; i < meta.size(); i++)
		keywords->addWord(meta[i]);
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
void DatabaseDataParser::addMetaWordsToData(GalleryData &data)
{
	string metaWords = "";
	vector<string> tokens = Utils::tokenize(data.path, "- \\");
	for (size_t i = 0; i < tokens.size(); i++)
	{
		if (keywords->searchWord(tokens[i]))
			data.keywords += (tokens[i] + ",");
	}
	if(!metaWords.empty())//remove the last comma since its not needed
		data.keywords.resize(data.keywords.length()-1); 
}
int DatabaseDataParser::getPartfOfSpeech(string word)
{
	//make these its own table
	return 0;
}
int DatabaseDataParser::getEntryIDFromDBTable(string table, string word)
{
	vector<string> *dbEntries = DBTables[table];
	for (size_t i = 0; i < dbEntries->size(); i++)
		if ((*dbEntries)[i] == word)
			return i;
	return -1;
}
//must be used with desciptive words only! otherwise we will need to add a check before, and i dont want to do that
bool DatabaseDataParser::isClothingNoun(string noun)
{
	//this will make sure we will only check values, not add non-existant ones, but this should be nesc if used properely
	//if (tableNamePartOfSpeech.count(noun))
	if (tableNamePartOfSpeech[noun] != NOUN)
		return false;
	if (noun != "Ethnicity" && noun != "Furniture" && noun != "Locations" && noun != "Bodypart" && noun != "Sextoys")
		return true;
	return false;
}
bool DatabaseDataParser::isConjunction(string word)
{
	if (word == "And" || word == "&")
		return true;
	return false;
}
//since this will be used after things are capitolized, all the checks will reflect that
//how to get this out of the code...put in cfg?
void DatabaseDataParser::transformClothingNameAlias(string &phrase)
{
	size_t found = phrase.find("School Girl");
	if (found != string::npos)
		phrase.replace(found, 11, "Schoolgirl");
	found = phrase.find("School-Girl");
	if (found != string::npos)
		phrase.replace(found, 11, "Schoolgirl");
	//knee highs can be socks or nylons....or even legwarmers
	found = phrase.find("Knee Highs");
	if (found != string::npos)
		phrase.replace(found, 9, "Kneehighsocks");
	found = phrase.find("Knee High Socks");
	if (found != string::npos)
		phrase.replace(found, 15, "Kneehighsocks");


	found = phrase.find("Thigh Highs");
	if (found != string::npos)
		phrase.replace(found, 10, "Thighhighsocks");

	found = phrase.find("Thigh High Socks");
	if (found != string::npos)
		phrase.replace(found, 16, "Thighhighsocks");


	found = phrase.find("Undies");
	if (found != string::npos)
		phrase.replace(found, 6, "Bra and Panties");

	found = phrase.find("T Shirt");
	if (found != string::npos)
		phrase.replace(found, 7, "Tshirt");
	found = phrase.find("T-Shirt");
	if (found != string::npos)
		phrase.replace(found, 7, "Tshirt");

	found = phrase.find("G-String");
	if (found != string::npos)
		phrase.replace(found, 8, "Gstring");

	found = phrase.find("Tank Top");
	if (found != string::npos)
		phrase.replace(found, 7, "Tanktop");

	found = phrase.find("Leg Warmers");
	if (found != string::npos)
		phrase.replace(found, 7, "Legwarmers");

	found = phrase.find("Crotchless Pantyhose");
	if (found != string::npos)
		phrase.replace(found, 7, "Crotchlesspantyhose");

	found = phrase.find("Seamless Pantyhose");
	if (found != string::npos)
		phrase.replace(found, 7, "Seamlesspantyhose");
}

void DatabaseDataParser::transformSexToyAlias(string &phrase)
{
	size_t found = phrase.find("Strap On");
	if (found != string::npos)
		phrase.replace(found, 8, "Strapon");
	
	found = phrase.find("Strap-on");
	if (found != string::npos)
		phrase.replace(found, 8, "Strapon");

	found = phrase.find("Butt Plug");
	if (found != string::npos)
		phrase.replace(found, 9, "Buttplug");

	found = phrase.find("Fucking Machine");
	if (found != string::npos)
		phrase.replace(found, 15, "Fuckingmachine");

	found = phrase.find("Anal Beads");
	if (found != string::npos)
		phrase.replace(found, 10, "Analbeads");
}

vector <ClothingItem> DatabaseDataParser::getOutfitFromGalleryName(string galleryName)
{
	transformClothingNameAlias(galleryName);
	vector <ClothingItem> clothes;
	vector<string> tokens = Utils::tokenize(galleryName, "- _");
	ClothingItem curItem;
	

	for (size_t i = 0; i < tokens.size(); i++)
	{
		string curWordDB = descriptiveWords[tokens[i]];
		if (!curWordDB.empty())
		{			
			//when we get a noun, we are probaly done describing cur object
			if (isClothingNoun(curWordDB))
			{
				curItem.type = curWordDB;
				curItem.index = getEntryIDFromDBTable(curWordDB, tokens[i]);
				clothes.push_back(curItem);
				//if our next word is an "and", we prob mean to extend the adjs to the next noun
				//since its size_t it wont ever be negative...so lets check to make sure the size > 1 first
				if (tokens.size() > 1 && i < (tokens.size() - 2))
				{
					if (isConjunction(tokens[i + 1]) && isClothingNoun(descriptiveWords[tokens[i + 2]]))
						continue;
					if (isClothingNoun(descriptiveWords[tokens[i + 1]]))
						continue;
				}
				curItem.clear();
			}
			else if (tableNamePartOfSpeech[curWordDB] == ADJ)
			{
				int index = getEntryIDFromDBTable(curWordDB, tokens[i]);
				if (index == -1)
					continue;
				else index++; //the db index starts at 1, not 0
				if (curWordDB == "Clothingmaterial")
					curItem.ClothingMaterialIndex = index;
				else if (curWordDB == "Clothingprint")
					curItem.ClothingPrintIndex = index;
				else if (curWordDB == "Colors")
					curItem.ColorIndex = index;
			}
		}
	}
	//this is for when we have some description for a noun we dont recognize...we still should collect the data we do know.
	//a black <blank> is better than no data at all
	if(!curItem.isEmpty())
		clothes.push_back(curItem);
	return clothes;
}
string DatabaseDataParser::getVerfiedWordFromGalleryName(string galleryName, string dbTableName)
{
	vector<string> tokens = Utils::tokenize(galleryName, "- _");

	for (size_t i = 0; i < tokens.size(); i++)
	{
		string curWordDB = descriptiveWords[tokens[i]];
		if (curWordDB == dbTableName)
			return  tokens[i];
	}

	return "";
}

//----------------------------------------------------------------------
bool DatabaseDataParser::hasDataFromTable(string data, map<int, string> dbValues)
{
	return false;
}
//----------------------------------------------------------------------
void DatabaseDataParser::getAllPaths(string path, vector<string> &dirsWithImages)
{
	fileWalker->takeDirSnapShot(path);
	fileWalker->getAllDirsWithImgs(dirsWithImages);
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
	
	//test clthing item dectetion from gallery name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\strap on threesome in black dress");//
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\blue-transparent-bikini");//
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\pink-fishnet-bikini");//
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\blonde in red tartan skirt white pantyhose");//
	
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\colege-girl-in-white-pantyhose-and-bra");//pantyhose and bra are both white, it should detect that
	
	//string replace tests
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\school-girl-in-white-pantyhose-and-bra");//school-gril should be turned into schoolgirl
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\strap-on sex");//
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\purple g-string");//
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\seamless pantyhose");//
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\someMag\\models\\fake person\\black lace undies");//

	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\59561");//sub website, no name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\models\\anita pearl\\black lace bustier black pattern pantyhose");//good sub website, 1 model
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\models\\Kristina Blond, Jennifer Stone and Thalia Festiny");//bad path sub website, 3 models, no gallery

	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls"); //bad format...not enough info
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\539\\fakeDir"); //bad format...no model name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\18-only-girls\\models\\anita e\\a-lonely\\fake dir");//bad format, too many dirs after model name
	testPaths.push_back("\\\\OPTIPLEX-745\\photos\\porno pics\\magazines\\swank\\leg action\\models\\Kristina Blond, Jennifer Stone and Thalia Festiny\\something fake");//sub website, 3 models

	printf("starting calcGalleryData test \n");

	for (size_t i = 0; i < testPaths.size(); i++)
	{
		GalleryData galleryData;
		printf("%s: ", testPaths[i].c_str());
		string error;
		bool ret = calcGalleryData(testPaths[i], "\\\\OPTIPLEX-745\\photos\\porno pics", galleryData,error);
		if (ret)
		{
			printf("\n");
			for (size_t k = 0; k < galleryData.models.size(); k++)
			{
				printf("model %d name:", k + 1);
				printf("%s %s %s\n", galleryData.models[k].name.firstName.c_str(), galleryData.models[k].name.middleName.c_str(), galleryData.models[k].name.lastName.c_str());
				
				
				if (galleryData.models[k].outfit.size() > 0)
				{
					printf("model %d first outfit:", k + 1);
					printf("%s\n", galleryData.models[k].outfit[0].type.c_str());
					/*printf("%d\n", galleryData.models[k].sexActionIndex);
					printf("%d\n", galleryData.models[k].hairColorIndex);*/
				}
			}
			printf("category:       %s\n", galleryData.category.c_str());
			printf("websiteName:    %s\n", galleryData.websiteName.c_str());
			printf("subWebsiteName: %s\n", galleryData.subWebsiteName.c_str());
			printf("galleryName:    %s\n", galleryData.galleryName.c_str());
			//printf("%s\n", galleryData.metaData.c_str());
			printf("\n");


		}
		else
			printf("its bad!\n");
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
	vector<string> allDirs = MyFileDirDll::dumpTreeToVector(true);
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