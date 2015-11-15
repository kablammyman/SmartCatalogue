//http://hackerlabs.org/blog/2012/07/30/organizing-photos-with-duplicate-and-similarity-checking/

//hasing code came from this virus infected site:
//http://www.developermemo.com/1262721/
#include "stdafx.h"

#include "similarImage.h"

#include "myFileDirDll.h"
#include <map>

#include <iostream>
#include <fstream>

//Hash  
string SimilarImage::HashValue(Mat &src)
{
	string rst(64, '\0');
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
	string rst(64, '\0');
	double dIdex[64];
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
			mean += dst.at<double>(i, j) / 64;
			++k;
		}
	}

	/* ??*/
	for (int i = 0; i<64; ++i)
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
	if ((str1.size() != 64) || (str2.size() != 64))
		return -1;
	int difference = 0;
	for (int i = 0; i<64; i++)
	{
		if (str1[i] != str2[i])
			difference++;
	}
	return difference;
}


int SimilarImage::test()
{
	Mat img = cv::imread("\\\\OPTIPLEX-745\\photos\\porno pics\\mega sites\\twistys\\models\\Valentina Nappi\\black flower robe blue flower undies\\8_628.jpg", cv::IMREAD_COLOR); 
	Mat img2 = cv::imread("\\\\OPTIPLEX-745\\photos\\porno pics\\mega sites\\twistys\\models\\Valentina Nappi\\black flower robe blue flower undies\\10_506.jpg", cv::IMREAD_COLOR);
	//cvNamedWindow("MyOpenCV");
	//cvShowImage("MyOpenCV", img);
	//cvWaitKey(0);
	//cvDestroyWindow("MyOpenCV");

	string hash = HashValue(img);
	string phash = pHashValue(img);
	string hash2 = HashValue(img2);
	string phash2 = pHashValue(img2);

	int dist = HanmingDistance(hash, hash2);
	int dist2 = HanmingDistance(phash, phash2);

	long long num = getIntValueOfHash(phash);
	long long num2 = getIntValueOfHash(phash2);


	return 0;
}

void SimilarImage::getAllImagePaths(string path, vector<string> & imgDirs)
{
	if (FileDir::MyFileDirDll::doesPathExist(path) == false)
		return;

	FileDir::MyFileDirDll::processDirectory(path);

	vector<string> allDirs = FileDir::MyFileDirDll::dumpTreeToVector(true);
	for (size_t i = 0; i < allDirs.size(); i++)
	{
		if (FileDir::MyFileDirDll::getNumFilesInDir(allDirs[i]) != 0)
			imgDirs.push_back(allDirs[i]);
	}		
}
long long SimilarImage::getIntValueOfHash(string bits)
{
	int multiplier = 1;
	long long counter = 0;
	for (int i = bits.size()-1; i > -1; i--)
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
void SimilarImage::findDupes(vector<string> & imgDirs, map<int,vector<int>> &dupeList)
{
	allImages.clear();
	for (size_t i = 0; i < imgDirs.size(); i++)
	{
		vector<string> Allfiles = FileDir::MyFileDirDll::getAllFileNamesInDir(imgDirs[i]);
		for (size_t j = 0; j < Allfiles.size(); j++)
		{
			Mat img = cv::imread(Allfiles[j], cv::IMREAD_COLOR);
			if (img.data == NULL)
				continue;
			ImageFiles newImage;
			newImage.hash = HashValue(img);
			newImage.phash = pHashValue(img);
			newImage.path = Allfiles[j];
			allImages.push_back(newImage);
		}
		
	}
	ofstream myfile;
	myfile.open("savedData.txt");
	for (size_t i = 0; i < allImages.size(); i++)
		myfile << allImages[i].ToString() << endl;
	
	myfile.close();
	//now all hashes have been calculated, time to start the compare

	//bool *alreadyIncludedMap = new bool[allImages.size()](); //the parens should set every value to false
	map<int, bool> alreadyIncludedMap;

	for (size_t i = 0; i < allImages.size(); i++)
	{
		for (size_t j = i; j < allImages.size(); j++)
		{
			if (i == j)
				continue;

			if (HanmingDistance(allImages[i].phash, allImages[j].phash) < 2)
			{			
				//if we found these 2 to be a match alraedy, dont add it to the list again!
				if (alreadyIncludedMap[i] && alreadyIncludedMap[j])
					continue;

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
