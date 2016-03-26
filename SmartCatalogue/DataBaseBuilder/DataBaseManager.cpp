// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>
//#include <thread>
#include <vector>

#include "DataBaseManager.h"
#include "DataBaseBuilder.h"

#include "WinToDBMiddleman.h"
#include "CFGReaderDll.h"
#include "CFGHelper.h"


//sc create "My Sample Service" binPath= C:\SampleService.exe


int main(int argc, char *argv[])
{
	CFGHelper::filePathBase = Utils::setProgramPath(argv[0]);
	CFGHelper::loadCFGFile();

	OutputDebugString("My Sample Service: Main: Entry");

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		string errorText = "My Sample Service: Main: StartServiceCtrlDispatcher returned error: "; 
		errorText += to_string(GetLastError());
		OutputDebugString(errorText.c_str());
		return -1;
	}

	OutputDebugString("My Sample Service: Main: Exit");
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

	OutputDebugString("My Sample Service: ServiceMain: Entry");

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		OutputDebugString("My Sample Service: ServiceMain: RegisterServiceCtrlHandler returned error");
		goto EXIT;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}

	//START STUFF
	cout << "starting server\n";
	conn.startServer(SOMAXCONN, CFGHelper::DataBaseManagerPort);
	
	

	OutputDebugString("My Sample Service: ServiceMain: Performing Service Start Operations");

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		OutputDebugString("My Sample Service: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString("My Sample Service: ServiceMain: SetServiceStatus returned error");
		}
		goto EXIT;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}

	// Start the thread that will perform the main task of the service

	hDirWatchThread = CreateThread(NULL, 0, doDirWatch, NULL, 0, NULL);
	hNetworkThread = CreateThread(NULL, 0, doNetWorkCommunication, NULL, 0, NULL);
	hMainThread = CreateThread(NULL, 0, doMainWorkerThread, NULL, 0, NULL);


	OutputDebugString("My Sample Service: ServiceMain: Waiting for Worker Thread to complete");

	// Wait until our main worker thread exits effectively signaling that the service needs to stop
	WaitForSingleObject(hMainThread, INFINITE);

	OutputDebugString("My Sample Service: ServiceMain: Worker Thread Stop Event signaled");

	/*
	* Perform any cleanup tasks
	*/
	OutputDebugString("My Sample Service: ServiceMain: Performing Cleanup Operations");

	conn.shutdown();
	CancelIo(hDir);
	int exitCode = 0;
	TerminateThread(hDirWatchThread, exitCode);
	//CancelIoEx(hDir, overlapped);

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		OutputDebugString("My Sample Service: ServiceMain: SetServiceStatus returned error");
	}

EXIT:
	OutputDebugString("My Sample Service: ServiceMain: Exit");

	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	OutputDebugString("My Sample Service: ServiceCtrlHandler: Entry");

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		OutputDebugString("My Sample Service: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		//STOP STUFF


		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			OutputDebugString("My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}

	OutputDebugString("My Sample Service: ServiceCtrlHandler: Exit");
}



//http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html
DWORD WINAPI doDirWatch(LPVOID lpParam)
{
	
	int nCounter = 0;
	FILE_NOTIFY_INFORMATION strFileNotifyInfo[1024];
	DWORD dwBytesReturned = 0;
	//LPOVERLAPPED overlapped;

	hDir = CreateFile(
		CFGHelper::pathToProcess.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
		);

	//main thread will close this down
	for (;;)
	{
		if (ReadDirectoryChangesW(hDir, (LPVOID)&strFileNotifyInfo, sizeof(strFileNotifyInfo), TRUE, FILE_NOTIFY_CHANGE_LAST_WRITE, &dwBytesReturned, NULL, NULL) == 0)
		{
			OutputDebugString("Reading Directory Change");
		}
		else
		{
			wchar_t* temp = strFileNotifyInfo[0].FileName;
			wstring msg = L"File Modified: ";
			wstring h = temp;
			msg += h;
			//OutputDebugString(msg.c_str());
			switch (strFileNotifyInfo[0].Action)
			{
				
			case FILE_ACTION_ADDED: //0x00000001
				//The file was added to the directory.
				break;
			case FILE_ACTION_REMOVED: //0x00000002
				//The file was removed from the directory.
				break;
			case FILE_ACTION_MODIFIED://0x00000003
				//The file was modified. This can be a change in the time stamp or attributes.
				break;
			case FILE_ACTION_RENAMED_OLD_NAME://0x00000004
				//The file was renamed and this is the old name.
				break;
			case FILE_ACTION_RENAMED_NEW_NAME://0x00000005
				//The file was renamed and this is the new name.
				break;
			}
		}
	}
}

DWORD WINAPI doNetWorkCommunication(LPVOID lpParam)
{
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int numConn = 0;

	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		//socket comm stuff
		conn.waitForClientAsync();

		numConn = (int)conn.getNumConnections();
		for (int i = 0; i < numConn; i++)
		{
			if (conn.hasRecivedData(i))
			{
				iResult = conn.getData(i, recvbuf, DEFAULT_BUFLEN);
				if (iResult > 0)
				{
					recvbuf[iResult] = '\0';
					//printf("%s -> %d bytes.\n", recvbuf, iResult);

					vector<string> argVec = Utils::tokenize(recvbuf, ",");
					CmdArg newCommand = parseCommand(argVec);

					newCommand.dest = i;
					tasks.push(newCommand);
				}
				//client disconnected
				else if (iResult == 0)
					conn.closeConnection(i);

				else
					printf("recv failed: %d\n", WSAGetLastError());
			}
		}
	}

	OutputDebugString("My Sample Service: network communication: Exit");

	return ERROR_SUCCESS;
}

DWORD WINAPI doMainWorkerThread(LPVOID lpParam)
{
	DatabaseBuilder dbBuilder(CFGHelper::dbPath, CFGHelper::ignorePattern);
	dbBuilder.FillMetaWords(CFGHelper::meta);
	
	vector<string> dbTableValues = CFG::CFGReaderDLL::getCfgListValue("tableNames");
	//if we cant find the table names in the cfg, thejust get out of here
	if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
	{
		cout << "couldnt find the list of table names in your cfg...\n";
		return -1;
	}

	dbBuilder.FillPartsOfSpeechTable(dbTableValues);
	//dbBuilder.verifyDB(CFGHelper::pathToProcess);
	
	OutputDebugString("My Sample Service: ServiceWorkerThread: Entry");

	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		if (tasks.empty())
			continue;

		CmdArg command = tasks.front();
		tasks.pop();
		
		if (command.cmd.empty())
			continue;
		else if (command.cmd == "-ADDGAL")
		{
			//these may not be needed anymore
			int goodDir = 0, badDir = 0, totalDir = 0;
			int start_s = clock();

			if (dbBuilder.AddDirToDB(command.data[0], true))
				goodDir++;
			else
				badDir++;

			totalDir++;

			string report = jobReport(start_s, totalDir, goodDir, badDir);
		}
		else if (command.cmd == "-DELGAL")
		{
			
		}
		else if (command.cmd == "-MOVGAL")
		{

		}
	}
	return 0;
}

/*


//these may not be needed anymore
int goodDir = 0, badDir = 0, totalDir = 0;
int start_s = clock();

//get the data AS i build the dir tree
MyFileDirDll::startDirTreeStep();
while (!MyFileDirDll::isFinished())
{
string curDir = MyFileDirDll::nextDirTreeStep(CFGHelper::pathToProcess);

if (dbBuilder.addDirToDB(curDir))
goodDir++;
else
badDir++;

totalDir++;
}
*/