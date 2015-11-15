#pragma once
#include <vector>
#include <string>

using namespace std;

//i guess shoes are included in this list...?
struct ClothingItem
{
	int clotingTypeIndex;
	int lingerieTypeIndex;
	int ClothingMaterialIndex;
	int ClothingPrintIndex;
	int ColorIndex;
	int BodyPartIndex;
};

struct ModelName
{
	string firstName;
	string lastName;
	string middleName; //for names like "Carmen Del Ray"
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
	int sexActionIndex;
	int hairColorIndex; //uses colors.color
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
};

