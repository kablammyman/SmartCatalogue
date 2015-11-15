#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <string>
#include <stdio.h>

class ModelFaceDB
{
	// Copy this file from opencv/data/haarscascades to target folder
	std::string face_cascade_name;
	cv::CascadeClassifier face_cascade;
	std::string window_name;
	FILE * pFile;
	std::string userName;
	std::string curSaveImgPath;
	int maxImages;


	bool dirExists(const std::string& dirName_in);

	std::string prepPathForTraversal(std::string path);
public:
	std::string filePathBase;
	bool detectAndDisplay(cv::Mat frame, int fileNum);
	ModelFaceDB();
	std::string getCurDirName(std::string path);
	void writeToCSV(std::string name, std::string filepath);
	int getFilesFromPath(std::string pathToTraverse);
	bool openCSV(std::string csvPath);

	int getFilesFromTextFile(std::string textFile);
};