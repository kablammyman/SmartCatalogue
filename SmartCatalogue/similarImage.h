#pragma once

#include <iostream>
#include <stdio.h>

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
public:
	vector<ImageFiles> allImages;

	//Hash  
	string HashValue(Mat &src);

	//pHash  
	string pHashValue(Mat &src);

	//  
	int HanmingDistance(string &str1, string &str2);

	int test();
	void loadCalculatedHashes();
	void getAllImagePaths(string path, vector<string> & imgDirs);
	long long getIntValueOfHash(string bits);
	void findDupes(vector<string> & imgDirs, map<int, vector<int>> &dupeList);
};