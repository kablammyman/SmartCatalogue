
#include "ImgAttribLookup.h"
#include "DBLookup.h"
#include "StringUtils.h"
#include <algorithm>    // std::set_intersection, std::sort

using namespace std;
QueryIDResults idResults;
std::vector<std::string> results;

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

 std::string(*QueryLookup[NUM_CHECKS]) (std::string name);

string GetCategoryID(string name)
{
	string output;
	string querey = "SELECT id FROM Category WHERE name = \""  + name +"\"";
	dbCtrlr.executeSQL(querey, output);
	dbCtrlr.removeTableNameFromOutput(output);
	idResults.CategoryID = output;

	return output;
}
string GetWebsiteID(std::string name)
{
	string output;
	StringUtils::ToProperNoun(name);
	string querey = "SELECT id FROM Website WHERE name = \"" + name + "\"";
	dbCtrlr.executeSQL(querey, output);
	dbCtrlr.removeTableNameFromOutput(output);
	idResults.WebsiteID = output;

	return output;
}
string GetSubWebsiteID(string name)
{
	string output;
	StringUtils::ToProperNoun(name);
	string querey = "SELECT id FROM SubWebsite WHERE name = \"" + name + "\"";
	dbCtrlr.executeSQL(querey, output);
	dbCtrlr.removeTableNameFromOutput(output);
	idResults.SubWebsiteID = output;

	return output;
}

string GetGalleryID(string name)
{
	string output;
	string querey = "Select ID from Gallery WHERE galleryName LIKE \"%" + name + "%\"";
	dbCtrlr.executeSQL(querey, output);
	dbCtrlr.getAllValuesFromCol(output, "ID", idResults.GalleryID);
	return name;
}


string GetModelID(string name)
{
	vector<string> names = StringUtils::Tokenize(name, ",&");
	vector<string> quereyResults;

	for (size_t i = 0; i < names.size(); i++)
	{
		string output;
		StringUtils::TrimWhiteSpace(names[i]);
		StringUtils::ToProperNoun(names[i]);
		vector<DatabaseController::dbDataPair> modelQuerey;
		vector<string> curFullName = StringUtils::Tokenize(names[i], " ");


		modelQuerey.push_back(make_pair("firstName", curFullName[0]));
		
		if(curFullName.size() == 2)
			modelQuerey.push_back(make_pair("lastName", curFullName[1]));

		else if (curFullName.size() == 3)
		{
			modelQuerey.push_back(make_pair("middleName", curFullName[1]));
			modelQuerey.push_back(make_pair("lastName", curFullName[2]));
		}
		else
		{
			modelQuerey.push_back(make_pair("middleName", ""));
			modelQuerey.push_back(make_pair("lastName", ""));
		}
		dbCtrlr.doDBQuerey("Models", "ID",modelQuerey, output);
		dbCtrlr.removeTableNameFromOutput(output);
		quereyResults.push_back(output);
	}
	
	idResults.ModelID = quereyResults;
	return quereyResults[0];
}

string GetMetaData(string name)
{
	return name;
}

void DoLookupFromIDs(vector<string> &results)
{
	vector<DatabaseController::dbDataPair> combinedIDQuerey;
	int idTypes = 0;

	//first, get teh galleries
	if(!idResults.CategoryID.empty())
		combinedIDQuerey.push_back(make_pair("CategoryID", idResults.CategoryID));
	if (!idResults.WebsiteID.empty())
		combinedIDQuerey.push_back(make_pair("WebsiteID", idResults.WebsiteID));
	if (!idResults.SubWebsiteID.empty())
		combinedIDQuerey.push_back(make_pair("SubWebsiteID", idResults.SubWebsiteID));

	vector<string> classifcationIDs;
	string combinedIDOutput;


	if (!combinedIDQuerey.empty())
	{
		idTypes++;
		dbCtrlr.doDBQuerey("Gallery", "ID", combinedIDQuerey, combinedIDOutput);
		dbCtrlr.getAllValuesFromCol(combinedIDOutput, "ID", classifcationIDs);
		
		if (!idResults.GalleryID.empty())
		{
			vector<string>::iterator it;
			vector<string> temp(idResults.GalleryID.size() + classifcationIDs.size());

			sort(classifcationIDs.begin(), classifcationIDs.end());
			sort(idResults.GalleryID.begin(), idResults.GalleryID.end());

			it = std::set_intersection(idResults.GalleryID.begin(), idResults.GalleryID.end(), classifcationIDs.begin(), classifcationIDs.end(), temp.begin());
			temp.resize(it - temp.begin());
			classifcationIDs.clear();
			classifcationIDs = temp;
		}

		
	}
	
	if (!idResults.GalleryID.empty() && combinedIDOutput.empty())
	{
		classifcationIDs.insert(classifcationIDs.end(), idResults.GalleryID.begin(), idResults.GalleryID.end());
		idTypes = 1;
	}


	//then look for the models in the galleries
	vector<string> modelGalleries;
	//i got some unintended behavior that i may want to keep in the future
	//this will take all garreis ech model is in and add them to the list, instead of only taking the common ones
	bool combineGalleries = false;
	if (!idResults.ModelID.empty())
	{
		for (size_t i = 0; i < idResults.ModelID.size(); i++)
		{
			idTypes++;
			DatabaseController::dbDataPair whereData;
			whereData.first = "ModelID";
			whereData.second = idResults.ModelID[i];
			string modelOutput;
			dbCtrlr.doDBQuerey("ModelsInGallery", "GalleryID", whereData, modelOutput);
			

			if(i == 0 || combineGalleries)
				dbCtrlr.getAllValuesFromCol(modelOutput, "galleryID", modelGalleries);
			else
			{
				vector<string> curModelGal;
				dbCtrlr.getAllValuesFromCol(modelOutput, "galleryID", curModelGal);
				
				//we found no galelries for this model, so theres no way a gallery with both models can exist
				if (curModelGal.empty())
					break;
				
				vector<string>::iterator it;
				vector<string> temp(modelGalleries.size() + curModelGal.size());

				sort(modelGalleries.begin(), modelGalleries.end());
				sort(curModelGal.begin(), curModelGal.end());

				it = std::set_intersection(curModelGal.begin(), curModelGal.end(), modelGalleries.begin(), modelGalleries.end(), temp.begin());
				temp.resize(it - temp.begin());
				modelGalleries.clear();
				modelGalleries = temp;
			}
			//if we found no models, and we arent combing galleries, then theres no need to continue
			if(modelGalleries.empty() && !combineGalleries)
				break;

			
		}


	}
	vector<string> allGalleries;
	vector<string> *galleryData;
	//if (!classifcationIDs.empty() && !modelGalleries.empty())
	if(idTypes == 2)
	{
		vector<string>::iterator it;
		allGalleries.resize(classifcationIDs.size() + modelGalleries.size());
		
		sort(modelGalleries.begin(), modelGalleries.end());
		sort(classifcationIDs.begin(), classifcationIDs.end());
		
		it = std::set_intersection(classifcationIDs.begin(), classifcationIDs.end(), modelGalleries.begin(), modelGalleries.end(), allGalleries.begin());
		allGalleries.resize(it - allGalleries.begin());
		galleryData = &allGalleries;
	}
	else if (!classifcationIDs.empty() && modelGalleries.empty())
		galleryData = &classifcationIDs;
	else 
		galleryData = &modelGalleries;

	if (galleryData->empty())
	{
		results.push_back("none");
		return;
	}
	string finalQuerey = " SELECT path  FROM Gallery WHERE ID in (";
	finalQuerey += (StringUtils::FlattenVector((*galleryData)) + ")");
	
	string finalOut;
	dbCtrlr.executeSQL(finalQuerey, finalOut);
	dbCtrlr.getAllValuesFromCol(finalOut, "path", results);


		
}


void CreateSelectOrDeleteBox(HWND hWnd)
{
	addOrDeleteBoxHandle = CreateDialog(mainInst, MAKEINTRESOURCE(IDD_QUICK_QUERY_BOX), hWnd, SelectOrDeleteQuereyBox);

	if (addOrDeleteBoxHandle == NULL)
	{
		int x = GetLastError();
		printf("error: %d", x);
	}
	ShowWindow(addOrDeleteBoxHandle, SW_SHOW);
}

void InitInputFields(HWND hDlg)
{
	QueryLookup[0] = GetCategoryID;
	QueryLookup[1] = GetWebsiteID;
	QueryLookup[2] = GetSubWebsiteID;
	QueryLookup[3] = GetModelID;
	QueryLookup[4] = GetGalleryID;
	//QueryLookup[6] = GetMetaData;


	int baseX = 5, baseY = 100, stepY = 32;
	int editSize = stepY - 2;
	int buttonWidth = (baseX * 2) + 100;

	SQLQureyTextHandle = hDlg;
	CreateWindow(TEXT("Edit"), TEXT("SELECT path FROM galleries where modelFirstName = \"\""), WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_BORDER, baseX, 30, 600, 20, hDlg, (HMENU)IDC_SQLQUEREY_TEXT, NULL, NULL);
	CreateWindow(TEXT("BUTTON"), TEXT("Querey"), WS_VISIBLE | WS_CHILD, baseX, 60, 100, 30, hDlg, (HMENU)IDC_QUERY_BTN, NULL, NULL);

	CheckBoxButtonGroup = CreateWindowEx(WS_EX_TRANSPARENT, TEXT("Button"), TEXT("Search Criteria"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX, 2, baseY, 290, 450, hDlg, (HMENU)11, NULL, NULL);
	categoryCheck = CreateWindow(TEXT("BUTTON"), TEXT("Category"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_CATERGORY_CHECK, NULL, NULL);
	categoryText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY), 1000, editSize, CheckBoxButtonGroup, (HMENU)IDC_CATERGORY_TEXT, NULL, NULL);

	websiteCheck = CreateWindow(TEXT("BUTTON"), TEXT("Website"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 2), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_WEBSITE_CHECK, NULL, NULL);
	websiteText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 2), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_WEBSITE_TEXT, NULL, NULL);

	subwebsiteCheck = CreateWindow(TEXT("BUTTON"), TEXT("sub Website"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 3), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_SUB_WEBSITE_CHECK, NULL, NULL);
	subwebsiteText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 3), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_SUB_WEBSITE_TEXT, NULL, NULL);

	modelCheck = CreateWindow(TEXT("BUTTON"), TEXT("Name(s)"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 4), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_MODEL_FIRST_NAME_CHECK, NULL, NULL);
	modelText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 4), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_MODEL_FIRST_NAME_TEXT, NULL, NULL);

	galleryCheck = CreateWindow(TEXT("BUTTON"), TEXT("gallery"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 5), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_GALLERY_CHECK, NULL, NULL);
	galleryText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 5), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_GALLERY_TEXT, NULL, NULL);

	//metaCheck = CreateWindow(TEXT("BUTTON"), TEXT("Meta data"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, baseX, baseY + (stepY * 7), buttonWidth, editSize, CheckBoxButtonGroup, (HMENU)IDC_META_CHECK, NULL, NULL);
	//metaText = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_BORDER, baseX * 25, baseY + (stepY * 7), 300, editSize, CheckBoxButtonGroup, (HMENU)IDC_META_TEXT, NULL, NULL);

	CreateWindow(TEXT("BUTTON"), TEXT("execute"), WS_VISIBLE | WS_CHILD, baseX, baseY + (stepY * 11), 100, 50, hDlg, (HMENU)IDC_EXECUTE_QUICK_QUERY_BTN, NULL, NULL);

}
BOOL CheckInput(WPARAM wParam, std::string &output)
{
	if (wParam == IDC_QUERY_BTN)
	{
		char buffer[MAX_PATH];

		int x = GetDlgItemText(SQLQureyTextHandle, IDC_SQLQUEREY_TEXT, buffer, MAX_PATH);
		if (x == 0)
		{
			x = GetLastError();
			printf("error: %d", x);
		}
		dbCtrlr.executeSQL(buffer, output);
		return (INT_PTR)TRUE;
	}
	else if(wParam == IDC_EXECUTE_QUICK_QUERY_BTN)
	{
		for (int i = 0; i < NUM_CHECKS; i++)
		{
			//if (IsDlgButtonChecked(CheckBoxButtonGroup, IDC_CATERGORY_CHECK + i))
			{
				char buffer[512] = { 0 };
				GetDlgItemText(CheckBoxButtonGroup, IDC_CATERGORY_TEXT + i, buffer, 512);
				if (buffer[0] == '\0')
					continue;

				(*QueryLookup[i]) (buffer);
			}
		}
		results.clear();
		DoLookupFromIDs(results);

		idResults.Clear();//we are done with this, so clear it for next use
		return TRUE;
	}

	return FALSE;
}
// Message handler for eitehr selecting or deleting record...window dims are in resource.rc
INT_PTR CALLBACK SelectOrDeleteQuereyBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{


	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:

		InitInputFields(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
	{
		std::string output;
		CheckInput(wParam, output);
	}
		break;
	case WM_PAINT:
		if (addOrDeleteBoxHandle != NULL)
			ShowWindow(addOrDeleteBoxHandle, SW_SHOW);
		break;
	case WM_CLOSE:
		EndDialog(hDlg, 0);
		break;
	}
	return (INT_PTR)FALSE;
}

