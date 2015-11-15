#include "stdafx.h"

#include <sstream>
#include <iostream>
#include <fstream>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "opencv2/core.hpp"
#include "opencv2/face.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/objdetect.hpp"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>


using namespace cv::face;
using namespace cv;
using namespace std;

//<name, vec of images>
map<string, vector<string>> faceDatabase;
vector<int> labels;//i hate these, but they are needed for the algo
map<string, vector<string>>::iterator it;
map<int, string> intToNameTable;

vector<string> tokenize(string path, string delims)
{
	vector<string> returnVec;
	char *p = strtok(const_cast<char *>(path.c_str()), delims.c_str());
	while (p)
	{
		//printf ("Token: %s\n", p);
		returnVec.push_back(p);
		p = strtok(NULL, delims.c_str());
	}
	return returnVec;
}

static void read_csv(const string& filename, vector<Mat>& images, map<string, vector<string>> &db)
{
	std::ifstream file(filename.c_str(), ifstream::in);
	if (!file)
	{
		string error_message = "No valid input file was given, please check the given filename.";
		CV_Error(CV_StsBadArg, error_message);
	}
	string line, path, oldName = "none";
	int counter = -1;
	//you need more than 1 person/images to make the algo work, and images must be the same size
	while (getline(file, line))
	{
		vector<string> tokens = tokenize(line, ",");
		//token 0 = name, tone 1 = an img path
		db[tokens[0]].push_back(tokens[1]);

		if (oldName.compare(tokens[0]) != 0)
			counter++;

		oldName = tokens[0];
		images.push_back(imread(tokens[1], 0));

		labels.push_back(counter);
		intToNameTable[counter] = tokens[0];
	}
}


int doTheThing(int argc, const char *argv[]) {
	// Check for valid command line arguments, print usage
	// if no arguments were given.
	//if (argc != 4) {
	//    cout << "usage: " << argv[0] << " </path/to/haar_cascade> </path/to/csv.ext> </path/to/device id>" << endl;
	//    cout << "\t </path/to/haar_cascade> -- Path to the Haar Cascade for face detection." << endl;
	//    cout << "\t </path/to/csv.ext> -- Path to the CSV file with the face database." << endl;
	//    cout << "\t <device id> -- The webcam device id to grab frames from." << endl;
	//    exit(1);
	//}
	//// Get the path to your CSV:
	//string fn_haar = string(argv[1]);
	//string fn_csv = string(argv[2]);
	//int deviceId = atoi(argv[3]);
	//// Get the path to your CSV:
	// please set the correct path based on your folder

	//string fn_haar = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_default.xml";
	string fn_haar = "C:\\opencv\\sources\\data\\haarcascades\\haarcascade_frontalface_alt.xml";
	string fn_csv = "G:\\programming\\my tools\\openCVTest\\Debug\\csv.txt";

	//I added these for prediuctiona nd confidence
	double confidence = 0.0;
	int predictionLabel = -1;

	int deviceId = 0;			// here is my webcam Id. 
								// These vectors hold the images and corresponding labels:
	vector<Mat> images;
	// Read in the data (fails if no valid input filename is given, but you'll get an error message):
	try {
		read_csv(fn_csv, images, faceDatabase);
	}
	catch (cv::Exception& e) {
		cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
		// nothing more we can do
		exit(1);
	}
	// Get the height from the first image. We'll need this
	// later in code to reshape the images to their original
	// size AND we need to reshape incoming faces to this size:
	int im_width = images[0].cols;
	int im_height = images[0].rows;

	// Create a FaceRecognizer and train it on the given images:
	Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
	model->train(images, labels);
	// That's it for learning the Face Recognition model. You now
	// need to create the classifier for the task of Face Detection.
	// We are going to use the haar cascade you have specified in the
	// command line arguments:
	//
	CascadeClassifier haar_cascade;
	haar_cascade.load(fn_haar);

	Mat frame;

	if (argc < 2)
		//imread("G:\\programming\\my tools\\openCVTest\\Debug\\rawImages\\Maggie Ross\\pp_Amateurs20120617_TheUnattainableBeauty_P1_Maggie_Ross_02-XL.jpg", IMREAD_COLOR); // Read the file
		imread("\\\\OPTIPLEX-745\\photos\\porno pics\\teens\\models\\bryci\\stripping-pink\\5_583.jpg", IMREAD_COLOR); // Read the file
	else
		frame = imread(argv[1], IMREAD_COLOR); // Read the file

	if (!frame.data) // Check for invalid input
	{
		cout << "Could not open or find the image" << argv[1] << std::endl;
		return -1;
	}

	namedWindow("Display window", WINDOW_AUTOSIZE); // Create a window for display.
	imshow("Display window", frame); // Show our image inside it.

									 // Clone the current frame:
	Mat original = frame.clone();
	// Convert the current frame to grayscale:
	Mat gray;
	cvtColor(original, gray, CV_BGR2GRAY);
	// Find the faces in the frame:
	vector< Rect_<int> > faces;
	haar_cascade.detectMultiScale(gray, faces);
	// At this point you have the position of the faces in
	// faces. Now we'll get the faces, make a prediction and
	// annotate it in the video. Cool or what?
	for (size_t i = 0; i < faces.size(); i++) {
		// Process face by face:
		Rect face_i = faces[i];
		// Crop the face from the image. So simple with OpenCV C++:
		Mat face = gray(face_i);
		// Resizing the face is necessary for Eigenfaces and Fisherfaces. You can easily
		// verify this, by reading through the face recognition tutorial coming with OpenCV.
		// Resizing IS NOT NEEDED for Local Binary Patterns Histograms, so preparing the
		// input data really depends on the algorithm used.
		//
		// I strongly encourage you to play around with the algorithms. See which work best
		// in your scenario, LBPH should always be a contender for robust face recognition.
		//
		// Since I am showing the Fisherfaces algorithm here, I also show how to resize the
		// face you have just found:
		Mat face_resized;
		cv::resize(face, face_resized, Size(im_width, im_height), 1.0, 1.0, INTER_CUBIC);
		// Now perform the prediction, see how easy that is:

		confidence = 0.0;
		predictionLabel = -1;

		//predictionLabel = model->predict(face_resized);
		model->predict(face_resized, predictionLabel, confidence);
		// And finally write all we've found out to the original image!
		// First of all draw a green rectangle around the detected face:
		rectangle(original, face_i, CV_RGB(0, 255, 0), 1);
		// Create the text we will annotate the box with:
		string box_text;
		box_text = format("Prediction = ");
		// Get stringname
		if (intToNameTable.find(predictionLabel) != intToNameTable.end())
			box_text.append(intToNameTable[predictionLabel]);

		else box_text.append("Unknown");

		box_text.append(" confidence: ");
		char str[8];
		sprintf(str, "%.2f", confidence);
		box_text.append(str);

		// Calculate the position for annotated text (make sure we don't
		// put illegal values in there):
		int pos_x = std::max(face_i.tl().x - 10, 0);
		int pos_y = std::max(face_i.tl().y - 10, 0);
		// And now put it into the image:
		putText(original, box_text, Point(pos_x, pos_y), FONT_HERSHEY_PLAIN, 1.0f, CV_RGB(0, 255, 0), 2.0f);
	}
	// Show the result:
	imshow("face_recognizer", original);
	// And display it:
	char key = (char)waitKey(20);
	// Exit this loop on escape:

	return 0;
}