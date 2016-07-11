#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <Windows.h>
#include <queue>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "CFGHelper.h"
#include "TCPUtils.h"
#include "CommandLineUtils.h"
#include "LogRouter.h"

#define SERVICE_NAME  "PornoDB Manager"
#define CREATE_HASH_OUTPUT_IGNORE "***** VIDEOINPUT LIBRARY - 0.1995 - TFW07 *****\n"


SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;

HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;
HANDLE				  hDir = NULL;

//i need to be able to start CerateUmageHash.exe, and this handle will let me know if i did that already
HANDLE				  hCreateImageHashProc = NULL;


TCPUtils *tcpOut = NULL;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
int doNetWorkCommunication(void);
int doWaitForRemoteConnections(void);
int doDirWatch(void);
int doMainWorkerThread(void);

queue<CmdArg> tasks;
TCPUtils conn;
LogRouter logRouter;

bool isService = true;
int createImageHashSocket = -1;

using namespace std;
thread	* networkThread;
thread	* dirWatchThread;
thread	* waitForClientThread;
thread	* mainThread;
thread	* createImageHashProc;
mutex              g_lockqueue;
condition_variable g_queuecheck;
//condition_variable g_connectionCheck;
bool               g_notified;
bool               g_hasConnections;
void invalidParmMessageAndExit()
{
	cout << "invlaid parameters. You need to provide a path to the DB and a path where your images are\n";
	cout << "DataBaseManager -dbPath \"C:\\somePath\\\" -dataPath \"C:\\photos\\myFunPhotos\\\"\n";
}

CmdArg parseCommand(vector<string> argv)
{
	size_t i = 0;
	CmdArg command;
	//std::vector<std::string> arguments(argv + 1, argv + argc);
	while (i < argv.size())
	{
		string cmdArgUpper = argv[i];

		for (size_t j = 0; j < cmdArgUpper.size(); j++)
			cmdArgUpper[j] = toupper(cmdArgUpper[j]);
			

		if (cmdArgUpper == "-DEST")
		{
			command.SetCommand(cmdArgUpper);
			i++;
			if (i >= argv.size())
			{
				command.clear();
				return command;
			}
			command.dest = atoi(argv[i].c_str());
		}
		else if (cmdArgUpper == "-ADDGAL" || cmdArgUpper ==  "-DELGAL" || cmdArgUpper == "-MOVGAL" || cmdArgUpper == "-VERIFYGAL" ||
			cmdArgUpper == "-ADDIMG" || cmdArgUpper == "-DELIMG" || cmdArgUpper == "-MOVIMG" || cmdArgUpper == "-VERIFYIMG")
		{
			command.SetCommand(cmdArgUpper);
			i++;
			if (i >= argv.size())
			{
				command.clear();
				return command;
			}
			command.data.push_back(argv[i]);
		}
	
		//this should be an image hash returning from over the wire
		else if(cmdArgUpper.find("HASH") != string ::npos)
		{
			i++;
			command.SetCommand("-HASH");
			for (size_t j = i; j < argv.size(); j++)
				command.data.push_back(argv[j]);

			break;
		}
		else
		{
			i++;
			command.SetCommand(cmdArgUpper);
		}
		i++;
	}
	//if we get here, we prob turned on/off server mode and thats it
	return command;
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
bool myCreateProcess(string pathAndName, string args, HANDLE &processCreated)
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
		return false;
	}

	processCreated = pi.hProcess;

	return true;
}

string jobReport(double start_s, int totalDir, int goodDir, int badDir)
{
	int stop_s = clock();
	double milis = (stop_s - start_s) / double(CLOCKS_PER_SEC) * 1000;
	string returnString = "done in " + LogOutput::GetTimeStampFromMilis(milis)+ "\n";
	returnString += "processed " + to_string(totalDir) + " galleries\n";

	float goodPercent = ((float)goodDir / (float)totalDir);
	goodPercent *= 100;
	returnString += "\nGalleries added to DB: " + to_string(goodDir) + " (" + to_string(goodPercent) + "%)\n";
	float badPercent = ((float)badDir / (float)totalDir);
	badPercent *= 100;
	returnString += "\nGalleries that dont conform to schema: " + to_string(badDir) + " (" + to_string(badPercent) + "%)\n";

	return returnString;
}

void StartAndConnectToCreateImageHash()
{
	unsigned long exitCode;
	GetExitCodeProcess(hCreateImageHashProc, &exitCode);
	
	if(exitCode == STILL_ACTIVE)
		return;

	myCreateProcess(CFGHelper::filePathBase+"\\CreateImageHash.exe", " -server", hCreateImageHashProc);
	//sleep a bit, so we make sure its running
	Sleep(1000);

	//now try to connect to it
	createImageHashSocket = conn.connectToServer(CFGHelper::CreateImageHashIP, CFGHelper::CreateImageHashPort);
	g_hasConnections = true;
	networkThread = new thread(doNetWorkCommunication); 
}

void ShutdownCreateImageHash()
{
	if(hCreateImageHashProc == NULL)
		return;
	
	conn.sendData(createImageHashSocket,"-exit");
	createImageHashSocket = -1;
	hCreateImageHashProc = NULL;
}


/*void DebugPrint(string msg)
{
	if(isService)
		OutputDebugString(msg.c_str());
	else
		cout << msg <<endl;
}*/

//when we have out put, does it go to the screen, a log, or back over the wire
void SendReportBackOriginator(string msg, int dest)
{
	if(dest == -1)
		logRouter.Log(LOG_LEVEL_INFORMATIONAL, msg);
	else
		conn.sendData(dest, msg.c_str());
}