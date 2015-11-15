
#include "stdafx.h"
#include <iostream>
#include <stdio.h>
#include "Windows.h"
#include <stack>
#include <fstream>
#include "detectAndSaveFacesJPG.h"
#include <map>

using namespace std;
using namespace cv;

int opencvTestFn(HWND parentWindow, string pathToImage = "")
{
	Mat image;
	string windowName = "Display window";
	if (pathToImage.empty())
		imread("G:\\001.jpg", IMREAD_COLOR); // Read the file
	else
		image = imread(pathToImage.c_str(), IMREAD_COLOR); // Read the file

	if (!image.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << std::endl;
		return -1;
	}

	namedWindow(windowName.c_str(), WINDOW_AUTOSIZE); // Create a window for display.
	HWND hWnd2 = (HWND)cvGetWindowHandle(windowName.c_str());
	//::SetParent(hWnd2,parentWindow); //this puts it in main window
	imshow(windowName.c_str(), image); // Show our image inside it.

	return 0;

}

ModelFaceDB::ModelFaceDB()
{
	// Copy this file from opencv/data/haarscascades to target folder
	face_cascade_name = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml";
	face_cascade;
	window_name = "Capture - Face detection";
	maxImages = 20;
}
bool ModelFaceDB::dirExists(const std::string& dirName_in)
{
  DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES)
    return false;  //something is wrong with your path!

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
    return true;   // this is a directory!

  return false;    // this is not a directory!

}

string ModelFaceDB::prepPathForTraversal(string path)
{
	char lastChar = *path.rbegin();
	string newPath = path;
	//if the path is already prepped, then do nothing
	if(lastChar == '*')
		return path;

	else if(lastChar == '\\')
		newPath += "*";//add asterisk to look for all "files" in cur dir
	else
		newPath += "\\*";//if we forgot to add the last back slash, add it, so we dont crash
	return  newPath;
}

string ModelFaceDB::getCurDirName(string path)
{
	size_t endOfName = path.rfind('\\');
	endOfName--;
	size_t startOfName = (path.rfind('\\',endOfName))+1;
	string temp = path.substr(startOfName,(endOfName-startOfName)+1);
	return temp;
}
  

void ModelFaceDB::writeToCSV(string name, string filepath)
{
	fprintf(pFile,"%s,%s\n",name.c_str(),filepath.c_str());
}

// Function detectAndDisplay
bool ModelFaceDB::detectAndDisplay(Mat frame, int fileNum)
{
    bool showImage = false;
	std::vector<Rect> faces;
    Mat frame_gray;
    Mat crop;
    Mat res;
    Mat gray;
    string text;
    stringstream sstm;

    //cvtColor(frame, frame_gray, COLOR_BGR2GRAY);
    //equalizeHist(frame_gray, frame_gray);
	frame_gray = frame;

// Detect faces
    face_cascade.detectMultiScale(frame_gray, faces, 1.1, 2, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

// Set Region of Interest
    cv::Rect roi_b;
    cv::Rect roi_c;

    size_t ic = 0; // ic is index of current element
    int ac = 0; // ac is area of current element

    size_t ib = 0; // ib is index of biggest element
    int ab = 0; // ab is area of biggest element
	string filename;

	if(faces.size() <= 0)
		return false;

    for (ic = 0; ic < faces.size(); ic++) // Iterate through all current elements (detected faces)
    {
        roi_c.x = faces[ic].x;
        roi_c.y = faces[ic].y;
        roi_c.width = (faces[ic].width);
        roi_c.height = (faces[ic].height);

        ac = roi_c.width * roi_c.height; // Get the area of current element (detected face)

        roi_b.x = faces[ib].x;
        roi_b.y = faces[ib].y;
        roi_b.width = (faces[ib].width);
        roi_b.height = (faces[ib].height);

        ab = roi_b.width * roi_b.height; // Get the area of biggest element, at beginning it is same as "current" element

        if (ac > ab)
        {
            ib = ic;
            roi_b.x = faces[ib].x;
            roi_b.y = faces[ib].y;
            roi_b.width = (faces[ib].width);
            roi_b.height = (faces[ib].height);
        }

        crop = frame(roi_b);
        resize(crop, res, Size(128, 128), 0, 0, INTER_LINEAR); // This will be needed later while saving images
        cvtColor(res, gray, CV_BGR2GRAY); // Convert cropped image to Grayscale

        // Form a filename
        filename = "";
        stringstream ssfn;
        ssfn << curSaveImgPath << fileNum << ".jpg";
        filename = ssfn.str();

        if(!imwrite(filename, gray))
		{
			printf("failed to save: %s\n",filename.c_str());
			return false;
		}
		writeToCSV(userName, filename);

		if(showImage)
		{
			Point pt1(faces[ic].x, faces[ic].y); // Display detected faces on main window - live stream from camera
			Point pt2((faces[ic].x + faces[ic].height), (faces[ic].y + faces[ic].width));
			rectangle(frame, pt1, pt2, Scalar(0, 255, 0), 2, 8, 0);
		}
    }

	if(showImage)
	{
		sstm << "Crop area size: " << roi_b.width << "x" << roi_b.height << " Filename: " << filename;
		text = sstm.str();

		putText(frame, text, cvPoint(30, 30), FONT_HERSHEY_COMPLEX_SMALL, 0.8, cvScalar(0, 0, 255), 1, CV_AA);
		imshow("original", frame);

		if (!crop.empty())
		{
			imshow("detected", crop);
		}
		else
        destroyWindow("detected");
	}
	return true;
}

int ModelFaceDB::getFilesFromPath(string pathToTraverse)
{
	   // Load the cascade
    if (!face_cascade.load(face_cascade_name))
    {
        printf("--(!)Error loading\n");
        return (-1);
    };

	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
		
	stack<string> dirStack;

	dirStack.push(pathToTraverse);
	int numFiles = 0;


	do{
		string curDir = dirStack.top(); //get the next dr to process
		string curName;
		dirStack.pop(); //take it off of stack
		
		curDir = prepPathForTraversal(curDir);

		hFind = FindFirstFile(curDir.c_str(), &FindFileData);

		size_t removeAst = curDir.length()-1;
		curDir.resize (removeAst); //curDir[removeAst]='\0';//remove the asterisk

		userName = getCurDirName(curDir);
		curSaveImgPath = (filePathBase + userName+"\\");
		if(!dirExists(curSaveImgPath))
			CreateDirectory(curSaveImgPath.c_str(),NULL);

		if(hFind != INVALID_HANDLE_VALUE)
		do{
			if( strcmp(FindFileData.cFileName, ".") == 0 ||  strcmp(FindFileData.cFileName, "..")==0)//ignore the cur and back dir symbols
				continue;
 
		//if we find a directory, add its name to the stack, so we can parse it later
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				string newPath = curDir + FindFileData.cFileName + "\\";
				dirStack.push(newPath);
			}
			//else this is a file, so add it to the list of files for this dir
			else
			{
				numFiles++;
				Mat curImg;
				string imgPath = curDir;
				imgPath += FindFileData.cFileName;
				curImg = imread(imgPath, IMREAD_COLOR); // Read the file
				if(!curImg.data ) // Check for invalid input
				{
					cout << "Could not open or find the image " << imgPath << std::endl ;
					continue;
				}
				detectAndDisplay(curImg,numFiles);
			}

		}while(FindNextFile(hFind, &FindFileData));
		

	}while(!dirStack.empty());
	fclose (pFile);
	FindClose(hFind);

	return 0;
}
//temp funtion to see if there are 2 models in the images, if so, dont use the pics since we wont be able to say which is which
bool has2Models(string path)
{
	size_t found = path.rfind("models");//sometimes we have 2 of these, so find the last one
	
	if(found == string::npos)
		return false;
	
	string nameBlock = path.substr(found+7);
	size_t end = nameBlock.find("\\");
	string names = nameBlock.substr(0,end);

	if(names.find("&") == string::npos && names.find(" and ") == string::npos)
		return false;
	return true;
}
/*
after running this, you will prob have to get rid of the bad faces...dpo a windwos search for files htat are less than 3.7kb (or somethign like that)
to find all non faces. this will also find some legit faces, but mostly bad
System.Size:<3.7kb
*/
int ModelFaceDB::getFilesFromTextFile(string textFile)
{
	   // Load the cascade
    if (!face_cascade.load(face_cascade_name))
    {
        printf("--(!)Error loading face cascade\n");
        return -1;
    };

	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	ifstream myfile;
	std::map<string,int> modelMap;

	string curDir;
	string line;
	string curName;
	string faceDbPath = filePathBase + "faceDB\\";

	myfile.open(textFile);
	
	if (!myfile.is_open())
		return -1;
	
	if(!dirExists(faceDbPath))
		CreateDirectory(faceDbPath.c_str(),NULL);


	//we ran this querey to get the file
	//SELECT modelFirstName,modelLastName,path FROM images WHERE modelFirstName IS NOT NULL
	while ( getline (myfile,line) )
    {
		if(line.find("modelFirstName") != std::string::npos)
		{
			size_t found=line.find('|');
			userName = line.substr(found+1);
			continue;
		}
		else if(line.find("modelLastName") != std::string::npos)
		{
			if(line.size() < 15) //14 = "modelLastName|"...dont add a space for an emty string
				continue;

			size_t found=line.find('|');
			userName += (" " + line.substr(found+1));
			
			int numFiles = modelMap[userName];
			if(numFiles <= 0)
				modelMap[userName] = 1;//input new model
			continue;
		}
		else if(line.find("path") != std::string::npos)
		{
			size_t found=line.find('|');
			curDir = line.substr(found+1);
			if(has2Models(curDir))
				continue;
			int numFiles = modelMap[userName];
			if(numFiles >= maxImages)//we only need about 15 pics or so, any more is useless, so skip
				continue;
		}
		else
			continue;

		curDir = prepPathForTraversal(curDir);
		
		curSaveImgPath = (faceDbPath+userName+"\\");


		if(!dirExists(curSaveImgPath))
			CreateDirectory(curSaveImgPath.c_str(),NULL);
		
		hFind = FindFirstFile(curDir.c_str(), &FindFileData);

		size_t removeAst = curDir.length()-1;
		curDir.resize (removeAst); //curDir[removeAst]='\0';//remove the asterisk

		if(hFind != INVALID_HANDLE_VALUE)
		do{
			if( strcmp(FindFileData.cFileName, ".") == 0 ||  strcmp(FindFileData.cFileName, "..")==0)//ignore the cur and back dir symbols
				continue;
 
		//if we find a directory, add its name to the stack, so we can parse it later
			if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{

			}
			//else this is a file, so add it to the list of files for this dir
			else
			{
				int numFiles = modelMap[userName];
				if(numFiles >= maxImages)//we only need about 15 pics or so, any more is useless, so skip
					break;

				Mat curImg;
				string imgPath = curDir;
				imgPath += FindFileData.cFileName;
				curImg = imread(imgPath, IMREAD_COLOR); // Read the file
				if(!curImg.data ) // Check for invalid input
				{
					cout << "Could not open or find the image " << imgPath << std::endl;
					continue;
				}
	
				if(detectAndDisplay(curImg,numFiles))
				{
					numFiles++;
					modelMap[userName] = numFiles;
				}
			}

		}while(FindNextFile(hFind, &FindFileData));
		
		
	}
	myfile.close();
	fclose (pFile);
	FindClose(hFind);

	return 0;
}


bool ModelFaceDB::openCSV(string csvPath)
{
	csvPath = (filePathBase + "csv.txt");
	pFile = fopen (csvPath.c_str(),"a");
	if (pFile==NULL)
	{
		printf("couldnt open file: %s",csvPath.c_str());
		return false;
	}
	return true;
}

