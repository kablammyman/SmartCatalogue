#pragma once
#include <vector>
#include <string>

using namespace std;

//i guess shoes are included in this list...?
struct ClothingItem
{
	string type; //clothing type, panty type, legwear, etc
	int index; //what index in its table can we find the actual name
	int ClothingMaterialIndex;
	int ClothingPrintIndex;
	int ColorIndex;
	int BodyPartIndex;
	//if type is empty, we can assume either nothing else is filled out or the data is junk
	// even if we have some aux info (like color), we dont know what this item is
	bool isEmpty()
	{
		return type.empty();
	}
	void clear()
	{
		type.clear();
		index = -1; 
		ClothingMaterialIndex = -1;
		ClothingPrintIndex = -1;
		ColorIndex = -1;
		BodyPartIndex = -1;
	}
	ClothingItem() :type(""), index(-1), ClothingMaterialIndex(-1), ClothingPrintIndex(-1), ColorIndex(-1), BodyPartIndex(-1)
	{}
};

struct ModelName
{
	string firstName;
	string lastName;
	string middleName; //for names like "Carmen Del Ray"
	int dbID = -1; //used when collecting\inserting data
};

//gernal model database
struct Model
{
	ModelName name;
	int EthnicityIndex;
	
	//Face face; //face recognition stuff
	//string notes; //known for pantyhose etc
};

//mode thats in this prticular gallery...
struct GalleryModel
{
	ModelName name;
	vector <ClothingItem> outfit;
	int sexActionIndex = 0;
	int hairColorIndex = 0; //uses colors.color
	
	/*
	int nude; // 0 = no, 1 = strips, 2 = started nude? 
	//do we want stuff for tats, breast side (to see if pre/post boob job)
	*/
};


struct GalleryData
{
	vector<GalleryModel> models;
	string toy;
	string location;
	string path;
	string websiteName;
	string subWebsiteName;
	string galleryName;
	string category;
	string keywords;
	int dbID = -1; //used when collecting\inserting data
};

