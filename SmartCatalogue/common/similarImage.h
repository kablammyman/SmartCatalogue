#pragma once

#include <iostream>
#include <stdio.h>
#include <utility>      // std::pair
#include <vector>
#include <map>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

struct ImageFiles
{
	string path;
	string hash;
	string phash;
	string ToString()
	{
		return (path + "," + hash + "," + phash);
	}
};


class SimilarImage
{
	int minHammingDist;
	int maxNumBits;
	
public:
	vector<ImageFiles> allImages; //maybe this should be private, but we need to see this list so we can display it
	//simple pair for when we are finding dupes not in our struct form
	typedef pair <string, string> hashPathPair;
	SimilarImage()
	{
		minHammingDist = 2;
		maxNumBits = 64;
	}

	//Hash  
	string HashValue(Mat &src);

	//pHash  
	string pHashValue(Mat &src);
	void setMinHammingDist(int x) { minHammingDist = x; }
	int getMinHammingDist() { return minHammingDist; }
	float getPercentDiff(int diff);
	int HanmingDistance(string &str1, string &str2);

	void calcImageHash(string imgPath);
	void calcImageHasesForDir(string imgDir);
	void dumpImageDataToTTextFile(string filename);
	int test(string imgPath1, string imgPath2, bool viewImg = false);
	int hashDistFrom2Images(string imgPath1, string imgPath2);
	int phashDistFrom2Images(string imgPath1, string imgPath2);

	void loadCalculatedHashes();
	void getAllImagePaths(string path, vector<string> & imgDirs);
	long long getIntValueOfHash(string bits);

	//supply the hashes in a vector...either the phash or normal hash
	void findDupes(vector<hashPathPair> & allHashes, map<int, vector<int>> &dupeList);
	//find the dupes in all the files we just hashed and are still in mem
	void findDupesFromMem(map<int, vector<int>> &dupeList);

	string getImageHash(string imgPath);
	string getImageHash(Mat imgData);
	string getImagePHash(string imgPath);
	string getImagePHash(Mat imgData);
	string getAllHash(string imgPath);
};