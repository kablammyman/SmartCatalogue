//http://hackerlabs.org/blog/2012/07/30/organizing-photos-with-duplicate-and-similarity-checking/

//hasing code came from this virus infected site:
//http://www.developermemo.com/1262721/
#include "stdafx.h"

#include "similarImage.h"

#include <iostream>
#include <fstream>

#include "myfiledirdll.h"

using namespace std;

//Hash  
string SimilarImage::HashValue(Mat &src)
{
	string rst(maxNumBits, '\0');
	Mat img;
	if (src.channels() == 3)
		cvtColor(src, img, CV_BGR2GRAY);
	else
		img = src.clone();
	/*??8x8?64,*/

	resize(img, img, Size(8, 8));
	/* ?(Color Reduce)??64?*/

	uchar *pData;
	for (int i = 0; i<img.rows; i++)
	{
		pData = img.ptr<uchar>(i);
		for (int j = 0; j<img.cols; j++)
		{
			pData[j] = pData[j] / 4;
		}
	}

	/* ?? 64?*/
	int average = (int)mean(img).val[0];

	/* ??  ??1,0*/
	Mat mask = (img >= (uchar)average);

	/* ??*/
	int index = 0;
	for (int i = 0; i<mask.rows; i++)
	{
		pData = mask.ptr<uchar>(i);
		for (int j = 0; j<mask.cols; j++)
		{
			if (pData[j] == 0)
				rst[index++] = '0';
			else
				rst[index++] = '1';
		}
	}
	return rst;
}

//pHash  
string SimilarImage::pHashValue(Mat &src)
{
	Mat img, dst;
	string rst(maxNumBits, '\0');
	double *dIdex = new double[maxNumBits];
	double mean = 0.0;
	int k = 0;
	if (src.channels() == 3)
	{
		cvtColor(src, src, CV_BGR2GRAY);
		img = Mat_<double>(src);
	}
	else
	{
		img = Mat_<double>(src);
	}

	/* ??Size(32,32)Size(8,8)??*/
	resize(img, img, Size(32, 32));

	/* ??DCT*/
	dct(img, dst);

	/* ?DCT?8*8DCT?*/
	for (int i = 0; i < 8; ++i) {
		for (int j = 0; j < 8; ++j)
		{
			dIdex[k] = dst.at<double>(i, j);
			mean += dst.at<double>(i, j) / maxNumBits;
			++k;
		}
	}

	/* ??*/
	for (int i = 0; i<maxNumBits; ++i)
	{
		if (dIdex[i] >= mean)
		{
			rst[i] = '1';
		}
		else
		{
			rst[i] = '0';
		}
	}
	return rst;
}

//  
int SimilarImage::HanmingDistance(string &str1, string &str2)
{
	if ((str1.size() != maxNumBits) || (str2.size() != maxNumBits))
		return -1;
	int difference = 0;
	for (int i = 0; i<maxNumBits; i++)
	{
		if (str1[i] != str2[i])
			difference++;
	}
	return difference;
}

float SimilarImage::getPercentDiff(int diff)
{
	float percent = (maxNumBits - diff);
	percent /= maxNumBits;
	percent *= 100;
	return percent;
}

int SimilarImage::test(string imgPath1, string imgPath2, bool viewImg)
{
	Mat img = cv::imread(imgPath1.c_str(), cv::IMREAD_COLOR);
	Mat img2 = cv::imread(imgPath2.c_str(), cv::IMREAD_COLOR);

	if (img.empty() || img2.empty())
	{
		cout << "invlaid image or path(s)\n";
		return -1;
	}

	if (viewImg)
	{
		cvNamedWindow("MyOpenCV");
		cvShowImage("MyOpenCV", imgPath1.c_str());
		cvWaitKey(0);
		cvDestroyWindow("MyOpenCV");
	}

	string hash = HashValue(img);
	string phash = pHashValue(img);
	string hash2 = HashValue(img2);
	string phash2 = pHashValue(img2);

	int dist = HanmingDistance(hash, hash2);
	int dist2 = HanmingDistance(phash, phash2);

	long long num = getIntValueOfHash(phash);
	long long num2 = getIntValueOfHash(phash2);

	cout << "hash: " << hash << endl;
	cout << "phash: " << phash << endl;
	cout << "hash2: " << hash2 << endl;
	cout << "phash2: " << phash2 << endl;
	cout << "dist: " << dist << endl;
	cout << "dist2: " << dist2 << endl;
	cout << "int value of hash1: " << num << endl;
	cout << "int value of hash2: " << num2 << endl;

	return 0;
}

int SimilarImage::hashDistFrom2Images(string imgPath1, string imgPath2)
{
	Mat img = cv::imread(imgPath1.c_str(), cv::IMREAD_COLOR);
	Mat img2 = cv::imread(imgPath2.c_str(), cv::IMREAD_COLOR);

	if (img.empty())
	{
		cout << "invlaid image or path: " << imgPath1;
		return -1;
	}
		
	if(img2.empty())
	{
		cout << "invlaid image or path: " << imgPath2;
		return -1;
	}

	string hash = HashValue(img);
	string hash2 = HashValue(img2);

	return HanmingDistance(hash, hash2);
}

int SimilarImage::phashDistFrom2Images(string imgPath1, string imgPath2)
{
	Mat img = cv::imread(imgPath1.c_str(), cv::IMREAD_COLOR);
	Mat img2 = cv::imread(imgPath2.c_str(), cv::IMREAD_COLOR);

	if (img.empty())
	{
		cout << "invlaid image or path: " << imgPath1;
		return -1;
	}

	if (img2.empty())
	{
		cout << "invlaid image or path: " << imgPath2;
		return -1;
	}

	string hash = pHashValue(img);
	string hash2 = pHashValue(img2);

	return HanmingDistance(hash, hash2);
}

void SimilarImage::getAllImagePaths(string path, vector<string> & imgDirs)
{
	if (MyFileDirDll::doesPathExist(path) == false)
		return;

	MyFileDirDll::addDirTree(path,10);

	vector<string> allDirs;
	MyFileDirDll::dumpTreeToVector(path, allDirs, true);
	for (size_t i = 0; i < allDirs.size(); i++)
	{
		if (MyFileDirDll::getNumFilesInDir(allDirs[i]) != 0)
			imgDirs.push_back(allDirs[i]);
	}		
}

long long SimilarImage::getIntValueOfHash(string bits)
{
	int multiplier = 1;
	long long counter = 0;
	for (size_t i = bits.size()-1; i > -1; i--)
	{
		if(bits[i] == '1')
			counter += multiplier;
		multiplier *= 2;
	}
	return counter;
}
void SimilarImage::loadCalculatedHashes()
{
	ifstream myfile;
	string line;
	myfile.open("savedData.txt");
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			size_t firstComma = line.find(",");
			size_t secondComma = line.rfind(",");

			if (firstComma == string::npos || (firstComma == secondComma))
				break;

			string path = line.substr(0, firstComma);
			string hash = line.substr(firstComma +1, (secondComma- firstComma)-1);
			string phash = line.substr(secondComma+1);
			ImageFiles newImage;
			newImage.phash = phash;
			newImage.hash = hash;
			newImage.path = path;
			allImages.push_back(newImage);

		}
	}

	myfile.close();
}

void SimilarImage::dumpImageDataToTTextFile(string filename)
{
	ofstream myfile;
	myfile.open(filename);
	for (size_t i = 0; i < allImages.size(); i++)
		myfile << allImages[i].ToString() << endl;

	myfile.close();
}

void SimilarImage::calcImageHash(string imgPath)
{
	Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
	if (img.data == NULL)
		return;
	ImageFiles newImage;
	newImage.hash = HashValue(img);
	newImage.phash = pHashValue(img);
	newImage.path = imgPath;
	//allImages.push_back(newImage);
}

string SimilarImage::getImageHash(string imgPath)
{
	Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
	if (img.data == NULL)
		return "";

	return HashValue(img);
}

string SimilarImage::getImageHash(Mat imgData)
{
	if (imgData.data == NULL)
		return "";

	return HashValue(imgData);
}

string SimilarImage::getImagePHash(string imgPath)
{
	Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
	if (img.data == NULL)
		return "";

	return pHashValue(img);

}

string SimilarImage::getImagePHash(Mat imgData)
{
	if (imgData.data == NULL)
		return "";

	return pHashValue(imgData);

}

void SimilarImage::calcImageHasesForDir(string imgDir)
{
	vector<string> Allfiles = MyFileDirDll::getAllFileNamesInDir(imgDir);
	for (size_t i = 0; i < Allfiles.size(); i++)
		calcImageHash(Allfiles[i]);
}

//supply the hashes in a vector...either the phash or normal hash
void SimilarImage::findDupes(vector<hashPathPair> & allHashes, map<int, vector<int>> &dupeList)
{
	map<int, bool> alreadyIncludedMap;
	for (int i = 0; i < allHashes.size(); i++)
	{
		for (int j = i; j < allHashes.size(); j++)
		{
			if (i == j)
				continue;
			//if we found these 2 to be a match alraedy, dont add it to the list again!
			if (alreadyIncludedMap[i] && alreadyIncludedMap[j])
				continue;

			if (HanmingDistance(allHashes[i].first, allHashes[j].first) < minHammingDist)
			{			
				//if we found these 2 to be a match alraedy, dont add it to the list again!
				/*if (alreadyIncludedMap[i] && alreadyIncludedMap[j])
					continue;*/

				vector<int> temp;
				if (dupeList.count(i) > 0)
					temp = dupeList[i];
				
				temp.push_back(j);
				dupeList[i] = temp;
	
				alreadyIncludedMap[i] = true;
				alreadyIncludedMap[j] = true;
			}
		}
	}
}

void SimilarImage::findDupesFromMem(map<int, vector<int>> &dupeList)
{
	map<int, bool> alreadyIncludedMap;
	for (int i = 0; i < allImages.size(); i++)
	{
		for (int j = i; j < allImages.size(); j++)
		{
			if (i == j)
				continue;
			//if we found these 2 to be a match alraedy, dont add it to the list again!
			if (alreadyIncludedMap[i] && alreadyIncludedMap[j])
				continue;

			if (HanmingDistance(allImages[i].phash, allImages[j].phash) < minHammingDist)
			{
				//if we found these 2 to be a match alraedy, dont add it to the list again!
				/*if (alreadyIncludedMap[i] && alreadyIncludedMap[j])
				continue;*/

				vector<int> temp;
				if (dupeList.count(i) > 0)
					temp = dupeList[i];

				temp.push_back(j);
				dupeList[i] = temp;

				alreadyIncludedMap[i] = true;
				alreadyIncludedMap[j] = true;
			}
		}
	}
}
