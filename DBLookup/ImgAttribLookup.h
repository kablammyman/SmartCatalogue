#pragma once

#include <string>
#include <vector>
#include <Windows.h>

#define NUM_CHECKS	5
struct QueryIDResults
{
	std::string CategoryID;
	std::string WebsiteID;
	std::string SubWebsiteID;
	std::vector<std::string> ModelID;
	std::vector<std::string> GalleryID;
	void Clear(void)
	{
		CategoryID.clear();
		WebsiteID.clear();
		SubWebsiteID.clear();
		ModelID.clear();
		GalleryID.clear();
	}
};


extern HWND addOrDeleteBoxHandle;
extern HWND SQLQureyTextHandle;
extern HWND RadioButtonGroup;
extern HWND CheckBoxButtonGroup;

extern HWND selectRadioButton;
extern HWND deleteRadioButton;
extern HWND categoryCheck;
extern HWND categoryText;

extern HWND websiteCheck;
extern HWND websiteText;

extern HWND subwebsiteCheck;
extern HWND subwebsiteText;

extern HWND modelCheck;
extern HWND modelText;

extern HWND galleryCheck;
extern HWND galleryText;

extern HWND metaCheck;
extern HWND metaText;
	
extern std::vector<std::string> results;
std::string GetCategoryID(std::string name);
std::string GetWebsiteID(std::string name);
std::string GetSubWebsiteID(std::string name);
std::string GetModelID(std::string name);
std::string GetGalleryID(std::string name);
std::string GetMetaData(std::string name);
	
INT_PTR CALLBACK SelectOrDeleteQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
void CreateSelectOrDeleteBox(HWND hWnd);
void InitInputFields(HWND hDlg);
BOOL CheckInput(WPARAM wParam, std::string &output);
void DoLookupFromIDs(std::vector<std::string> &results);
