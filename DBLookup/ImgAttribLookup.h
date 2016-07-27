#pragma once

#include <string>
#include <Windows.h>

#define NUM_CHECKS					7
class ImgAttribLookup
{
private:
	HWND addOrDeleteBoxHandle;
	HWND SQLQureyTextHandle;
	HWND RadioButtonGroup;
	HWND CheckBoxButtonGroup;

	HWND selectRadioButton;
	HWND deleteRadioButton;
	HWND categoryCheck;
	HWND categoryText;

	HWND websiteCheck;
	HWND websiteText;

	HWND subwebsiteCheck;
	HWND subwebsiteText;

	HWND modelCheck;
	HWND modelText;

	HWND galleryCheck;
	HWND galleryText;

	HWND metaCheck;
	HWND metaText;
	
	std::string GetCategoryID(std::string name);
	std::string GetWebsiteID(std::string name);
	std::string GetSubWebsiteID(std::string name);
	std::string GetModelID(std::string name);
	std::string GetGalleryID(std::string name);
	std::string GetMetaData(std::string name);
	
	std::string(ImgAttribLookup::*QueryLookup)(std::string name) const = NULL;
//http://stackoverflow.com/questions/1485983/calling-c-class-methods-via-a-function-pointer
	//std::string(*QueryLookup[NUM_CHECKS]) (std::string name);

public:
	INT_PTR CALLBACK SelectOrDeleteQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void CreateSelectOrDeleteBox(HWND hWnd);
	void InitInputFields(HWND hDlg);
	BOOL CheckInput(WPARAM wParam, std::string &output);

	//an array of funcation pointers to differnt queries

	
};

/*
cstyle function pointer assingment
std::string(*QueryLookup[NUM_CHECKS]) (std::string name);

QueryLookup[0] = GetCategoryID;
QueryLookup[1] = GetWebsiteID;
QueryLookup[2] = GetSubWebsiteID;
QueryLookup[3] = GetModelID;
QueryLookup[4] = GetGalleryID;
QueryLookup[6] = GetMetaData;

use:
(*QueryLookup[i]) (buffer);
*/