// readFromClipboard.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

using namespace cv;


std::string getClipboardStringData()
{
	if (!OpenClipboard(NULL))
	{
		printf("Can't open clipboard");
		return "";
	}

	HANDLE textData = GetClipboardData(CF_TEXT);
	std::string rturnstring ((char *)textData);

	CloseClipboard();
	return rturnstring;
}

// Hbitmap convert to IplImage
IplImage * hBitmapToIpl(HBITMAP hBmp)
{
	BITMAP bmp;
	if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp))
	{
		return NULL;
	}
	// Get channels which equal 1 2 3 or 4 BmBitsPixel:
	// Specifies the number of bits Required to indicate the color of a pixel.
	int nChannels = bmp.bmBitsPixel == 1 ? 1 : bmp.bmBitsPixel / 8;

	// Get depth color bitmap or grayscale
	int depth = bmp.bmBitsPixel == 1 ? IPL_DEPTH_1U : IPL_DEPTH_8U;


	// Create header image
		IplImage * img = cvCreateImage(cvSize(bmp.bmWidth, bmp.bmHeight), depth, nChannels);
		//IplImage * img = cvCreateImage(cvSize(533, 800), IPL_DEPTH_8U, 3);
	// Allocat memory for the pBuffer
		BYTE * pBuffer = new   BYTE[bmp.bmHeight * bmp.bmWidth * nChannels];
		//BYTE * pBuffer = new   BYTE[800 * 533 * 3];
	// Copies the bitmap bits of a specified device - dependent bitmap into a buffer
		GetBitmapBits(hBmp, bmp.bmHeight * bmp.bmWidth * nChannels, pBuffer);
		//GetBitmapBits(hBmp, 800 * 533 * 3, pBuffer);
	// Copy data to the imagedata
		memcpy(img->imageData, pBuffer, bmp.bmHeight * bmp.bmWidth * nChannels);
		//memcpy(img->imageData, pBuffer, 800 * 533 * 3);
	
		delete pBuffer;

	// Create the image
		IplImage * dst = cvCreateImage(cvGetSize(img), img->depth, 3);
	// Convert color
		cvCvtColor(img, dst, CV_BGRA2BGR);
	cvReleaseImage(&img);
	return dst;
}
int test()
{
	if (!OpenClipboard(NULL))
	{
		printf("Can't open clipboard");
		return -1;
	}

	if (IsClipboardFormatAvailable(CF_BITMAP) || IsClipboardFormatAvailable(CF_DIB) || IsClipboardFormatAvailable(CF_DIBV5))
	{
		HBITMAP hbmp = (HBITMAP)GetClipboardData(CF_BITMAP);

		if (hbmp != NULL && hbmp != INVALID_HANDLE_VALUE)
		{			
			IplImage *img = hBitmapToIpl(hbmp);
			cvShowImage("image", img);
			cvWaitKey();
			cvDestroyWindow("image");
		}
	}

	CloseClipboard();

	return 0;
}

