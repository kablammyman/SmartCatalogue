#pragma once

#include <iostream>
#include <stdio.h>

#include <vector>
#include <map>
#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"


#include "DatabaseController.h"

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
	DatabaseController dbCtrlr;
	int minHammingDist;
	
public:
	vector<ImageFiles> allImages; //maybe this should be private, but we need to see this list so we can display it
	SimilarImage()
	{
		minHammingDist = 2;
	}

	//Hash  
	string HashValue(Mat &src);

	//pHash  
	string pHashValue(Mat &src);

	//  
	int HanmingDistance(string &str1, string &str2);

	void calcImageHash(string imgPath);
	void calcImageHasesForDir(string imgDir);
	void dumpImageDataToTTextFile(string filename);
	int test(string imgPath1, string imgPath2, bool viewImg = false);
	void loadCalculatedHashes();
	void getAllImagePaths(string path, vector<string> & imgDirs);
	long long getIntValueOfHash(string bits);
	void findDupes(vector<string> & imgDirs, map<int, vector<int>> &dupeList);

	string getImageHash(string imgPath);
	string getImagePHash(string imgPath);
};