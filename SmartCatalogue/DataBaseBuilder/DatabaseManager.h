#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>//check execution time
#include <Windows.h>

#include "NetworkConnection.h"
#include "Utils.h"



#define SERVICE_NAME  "PornoDB Manager"
#define CREATE_HASH_OUTPUT_IGNORE "***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****\n"


SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;

HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
HANDLE				  hDir;
HANDLE				  hNetworkThread;
HANDLE				  hDirWatchThread;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI doNetWorkCommunication(LPVOID lpParam);
DWORD WINAPI doDirWatch(LPVOID lpParam);

NetworkConnection conn;

bool verboseOutput = false;
bool doImageHash = true;;

using namespace std;


void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\\somePath\\\" -dataPath \"C:\\photos\\myFunPhotos\\\"\n";
}

int parseCommand(string cmd)
{
	if (cmd == "time")
		return 0;
	else if (cmd == "name")
		return 1;

	return -1;
}
void connectToRemote()
{
	cout << "connecting to " << ip << "\n";
	if (conn.connectToServer(ip, port) == NETWORK_ERROR)
	{
		exit(-1);
	}
}

void broadcastMsg(string msg)
{
	cout << "sending msg \n";
	conn.ServerBroadcast(msg.c_str());
}

string exec(const char* cmd)
{
	FILE* pipe;
	pipe = _popen(cmd, "r");
	if (!pipe)
		return "ERROR";
	char buffer[128];
	string result = "";

	while (fgets(buffer, 128, pipe) != NULL)
	{
		//hack until i figure out how to get rid of this message
		//***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****
		if (strcmp(buffer, CREATE_HASH_OUTPUT_IGNORE) == 0)
			continue;
		if (strcmp(buffer, "\n") == 0)
			continue;
		result += buffer;
	}
	_pclose(pipe);
	return result;
}
//to use with args, there needs to be a space like so:
//myCreateProcess("C:\\windows\\notepad.exe", " example.txt");
void myCreateProcess(string pathAndName, string args)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (!
		CreateProcess(
			TEXT(pathAndName.c_str()),
			(LPSTR)args.c_str(),
			NULL, NULL, FALSE,
			CREATE_NEW_CONSOLE,
			NULL, NULL,
			&si,
			&pi
			)
		)
	{
		cout << "Unable to execute.";
	}
}

string jobReport(double start_s, int totalDir, int goodDir, int badDir)
{
	int stop_s = clock();
	double milis = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	string returnString = "done in " + Utils::getTimeStamp(milis)+ "\n";
	returnString += "processed " + to_string(totalDir) + " galleries\n";

	float goodPercent = ((float)goodDir / (float)totalDir);
	goodPercent *= 100;
	returnString += "\nGalleries added to DB: " + to_string(goodDir) + " (" + to_string(goodPercent) + "%)\n";
	float badPercent = ((float)badDir / (float)totalDir);
	badPercent *= 100;
	returnString += "\nGalleries that dont conform to schema: " + to_string(badDir) + " (" + to_string(badPercent) + "%)\n";

	return returnString;
}