// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <algorithm>
#include <string>
#include <windows.h>
#include <winsock.h>
#include <stdio.h>

#include <vector>

#include "DataBaseManager.h"
#include "DataBaseBuilder.h"

#include "WinToDBMiddleman.h"
#include "CFGReaderDll.h"
#include "CFGHelper.h"

#define SHORT_SLEEP 1
#define LONG_SLEEP 10000
//sc create "PornoDB Manager" binPath= C:\SampleService.exe

void Start()
{
	//START STUFF
	Sleep(10000);
	cout << "starting server\n";
	conn.startServer(SOMAXCONN, CFGHelper::DataBaseManagerPort);
	/*hDirWatchThread = CreateThread(NULL, 0, doDirWatch, NULL, 0, NULL);
	hNetworkThread = CreateThread(NULL, 0, doNetWorkCommunication, NULL, 0, NULL);
	hWaitForClientThread = CreateThread(NULL, 0, doWaitForRemoteConnections, NULL, 0, NULL);
	hMainThread = CreateThread(NULL, 0, doMainWorkerThread, NULL, 0, NULL);*/
	 networkThread = NULL;
	 dirWatchThread = new thread(doDirWatch);
	 waitForClientThread = new thread(doWaitForRemoteConnections);
	 mainThread = new thread(doMainWorkerThread);
	

}

void Finish()
{
	conn.shutdown();
	CancelIo(hDir);
	
	//std::terminate();

	//int exitCode = 0;
	//TerminateThread(dirWatchThread, exitCode);
	//CancelIoEx(hDir, overlapped);
}


int main(int argc, char *argv[])
{
	CFGHelper::filePathBase = Utils::setProgramPath(argv[0]);
	CFGHelper::loadCFGFile();
	string err;
	if (!CFGHelper::IsCFGComplete(err))
	{
		string msg = "CFG file incomplete: " + err;
		MessageBox(NULL, msg.c_str(), NULL, NULL);
		exit(0);
	}

	DebugPrint("PornoDB Manager: Main: Entry");

	if(argc >= 2)
		isService = false;

	//when debugging, this is a normal console exe
	if (!isService)
	{
		// Start the thread that will perform the main task of the service
		Start();

		DebugPrint("PornoDB Manager: ServiceMain: Waiting for Worker Thread to complete");


		// Wait until our main worker thread exits effectively signaling that the service needs to stop
		//WaitForSingleObject(hMainThread, INFINITE);
		mainThread->join();
		DebugPrint("PornoDB Manager: ServiceMain: Worker Thread Stop Event signaled");

		/*
		* Perform any cleanup tasks
		*/
		DebugPrint("PornoDB Manager: ServiceMain: Performing Cleanup Operations");

		Finish();
		return 0;
	}


	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		string errorText = "PornoDB Manager: Main: StartServiceCtrlDispatcher returned error: "; 
		errorText += to_string(GetLastError());
		DebugPrint(errorText.c_str());
		return -1;
	}

	DebugPrint("PornoDB Manager: Main: Exit");
	return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;
	
	DebugPrint("PornoDB Manager: ServiceMain: Entry");

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		DebugPrint("PornoDB Manager: ServiceMain: RegisterServiceCtrlHandler returned error");
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
		DebugPrint("PornoDB Manager: ServiceMain: SetServiceStatus returned error");
	}

	
	

	DebugPrint("PornoDB Manager: ServiceMain: Performing Service Start Operations");

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		DebugPrint("PornoDB Manager: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			DebugPrint("PornoDB Manager: ServiceMain: SetServiceStatus returned error");
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
		DebugPrint("PornoDB Manager: ServiceMain: SetServiceStatus returned error");
	}

	// Start the thread that will perform the main task of the service
	Start();

	DebugPrint("PornoDB Manager: ServiceMain: Waiting for Worker Thread to complete");

	// Wait until our main worker thread exits effectively signaling that the service needs to stop
	//WaitForSingleObject(hMainThread, INFINITE);
	mainThread->join();

	DebugPrint("PornoDB Manager: ServiceMain: Worker Thread Stop Event signaled");

	/*
	* Perform any cleanup tasks
	*/
	DebugPrint("PornoDB Manager: ServiceMain: Performing Cleanup Operations");

	Finish();

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		DebugPrint("PornoDB Manager: ServiceMain: SetServiceStatus returned error");
	}

EXIT:
	DebugPrint("PornoDB Manager: ServiceMain: Exit");

	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	DebugPrint("PornoDB Manager: ServiceCtrlHandler: Entry");

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		DebugPrint("PornoDB Manager: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			DebugPrint("PornoDB Manager: ServiceCtrlHandler: SetServiceStatus returned error");
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		//if we are waiting for the thign to do, then lets make sure we still can stop the service
		g_notified = true;
		g_queuecheck.notify_one();

		break;

	default:
		break;
	}

	DebugPrint("PornoDB Manager: ServiceCtrlHandler: Exit");
}



//http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html
int doDirWatch(void)
{
	
	int nCounter = 0;
	FILE_NOTIFY_INFORMATION strFileNotifyInfo[1024];
	DWORD dwBytesReturned = 0;
	//LPOVERLAPPED overlapped;
	//vector<CmdArg> commandStaging;
	//string dirPAth(CFGHelper::pathToProcess.begin(), CFGHelper::pathToProcess.end());
	
	hDir = CreateFileA(
		CFGHelper::pathToProcess.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
		);
	if (hDir == INVALID_HANDLE_VALUE)
	{
		int winErr = GetLastError();
		printf("error: %d\n", winErr);
	}


	//main thread will close this down
	int counter = 0;
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		int err = ReadDirectoryChangesW(hDir, (LPVOID)&strFileNotifyInfo, 
			sizeof(strFileNotifyInfo), 
			TRUE, 
			//FILE_NOTIFY_CHANGE_LAST_WRITE,// | //this seems to track everything and nothing at the same time!
			//FILE_NOTIFY_CHANGE_SIZE |  // in file or subdir
			//FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_DIR_NAME | // creating, deleting a directory or sub
			FILE_NOTIFY_CHANGE_FILE_NAME, // renaming,creating,deleting a file
			&dwBytesReturned, NULL, NULL);

		if(err != 0)
		{
			int len = strFileNotifyInfo[0].FileNameLength / sizeof(WCHAR);
			char *fileName = new char[len+1];
			
			wcstombs(fileName, strFileNotifyInfo[0].FileName, len);
			fileName[len] = '\0';
			counter++;
			
			bool isFile = false;
			CmdArg newCommand;
			for (size_t i = 0; i < len; i++)
			{
				if (fileName[i] == '.')
				{
					isFile = true;
					break;
				}
			}
			string fullPath(CFGHelper::pathToProcess + fileName);
			
			switch (strFileNotifyInfo[0].Action)
			{
				
			case FILE_ACTION_ADDED: //0x00000001
				//The file was added to the directory.
				if(isFile)
					newCommand.SetCommand("-ADDIMG");
				else
					newCommand.SetCommand("-ADDGAL");
				break;
			case FILE_ACTION_REMOVED: //0x00000002
				//The file was removed from the directory.
				newCommand.SetCommand("-DELGAL");
				break;
			case FILE_ACTION_MODIFIED://0x00000003
				//The file was modified. This can be a change in the time stamp or attributes.
				//newCommand.SetCommand("-ADDGAL");
				break;
			case FILE_ACTION_RENAMED_OLD_NAME://0x00000004
				//The file was renamed and this is the old name.
				newCommand.SetCommand("-DELGAL");
				break;
			case FILE_ACTION_RENAMED_NEW_NAME://0x00000005
				//The file was renamed and this is the new name.
				//newCommand.SetCommand("-ADDGAL");
				break;
			}

			newCommand.data.push_back(fullPath);
			
			delete fileName;
			fileName = NULL;
			//commandStaging.push_back(newCommand);


		//	unique_lock<mutex> locker(g_lockqueue);
			tasks.push(newCommand);
			//for some reason, it randomly triggers...so lets not deal with it
			/*for(size_t i = 0; i < commandStaging.size(); i++)
				if (commandStaging[i].cmd != "")
					tasks.push(commandStaging[i]);*/
			//counter = 0;

			//commandStaging.clear();
			g_notified = true;
			g_queuecheck.notify_one();
		}
		else
		{
			int winErr = GetLastError();
			printf("error: %d\n%d\n",err, winErr);
		}
	}
	return 0;
}
int doWaitForRemoteConnections(void)
{
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		conn.waitForFirstClientConnect();
		if(conn.getNumConnections() > 0)
			g_hasConnections = true;
	}	

	return 0;
}
int doNetWorkCommunication(void)
{
	int iResult;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	int numConn = 0;
	bool recvData = false;
	std::clock_t  heartBeat;
	//  Periodically check if the service has been requested to stop
	//while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{//socket comm stuff
		/*std::unique_lock<std::mutex> connLocker(g_lockConnection);
		while (!g_hasConnections)
		{
			g_connectionCheck.wait(connLocker);
		}*/
		
		while (g_hasConnections)
		{
			recvData = false;
			numConn = (int)conn.getNumConnections();
			if (numConn == 0)
			{
				g_hasConnections = false;
				break;
			}
			for (int i = 0; i < numConn; i++)
			{
				if (conn.hasRecivedData(i))
				{
					recvData = true;
					iResult = conn.getData(i, recvbuf, DEFAULT_BUFLEN);
					if (iResult > 0)
					{
						recvbuf[iResult-1] = '\0';
						//printf("%s -> %d bytes.\n", recvbuf, iResult);

						vector<string> argVec = Utils::tokenize(recvbuf, "|");
						CmdArg newCommand = parseCommand(argVec);

						newCommand.dest = i;
						
						//unique_lock<mutex> locker(g_lockqueue);
						tasks.push(newCommand);
						g_notified = true;
						g_queuecheck.notify_one();
						heartBeat = std::clock();
					}
					//client disconnected
					else if (iResult == 0)
					{
						conn.closeConnection(i);
					}
					else
						printf("recv failed: %d\n", WSAGetLastError());
				}
			}
			if (!recvData)
			{
				Sleep(SHORT_SLEEP);  
				float curTime = (std::clock() - heartBeat) / CLOCKS_PER_SEC;
				if(curTime > 600)
					ShutdownCreateImageHash();
			}
		}

	}

	DebugPrint("PornoDB Manager: network communication: Exit");
	networkThread = NULL;
	return ERROR_SUCCESS;
}

int doMainWorkerThread(void)
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
	
	DebugPrint("PornoDB Manager: ServiceWorkerThread: Entry");

	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		std::unique_lock<std::mutex> locker(g_lockqueue);
		while (!g_notified) // used to avoid spurious wakeups 
			g_queuecheck.wait(locker);
		
		while (!tasks.empty())
		{
			CmdArg command = tasks.front();
			tasks.pop();

			if (command.cmd.empty())
				continue;
			else if (command.cmd == "-ADDGAL")
			{
				StartAndConnectToCreateImageHash();

				int goodDir = 0, badDir = 0, totalDir = 0;
				int start_s = clock();

				if (dbBuilder.AddDirToDB(command.data[0], true))
				{
					//int galleryID = WinToDBMiddleman::GetGalleryIDFromPath(command.data[0]);
					//dbBuilder.RequestImageHashesForDir(command.data[0], galleryID, &conn, createImageHashSocket);
					goodDir++;
				}
				else
					badDir++;

				totalDir++;

				string report = jobReport(start_s, totalDir, goodDir, badDir);

				//ShutdownCreateImageHash();
			}
			else if (command.cmd == "-DELGAL")
			{
				string output;
				WinToDBMiddleman::DeleteGalleryFromDB(command.data[0], output);
			}
			else if (command.cmd == "-MOVGAL")
			{
				WinToDBMiddleman::MoveGalleryOnDB(command.data[0], command.data[1]);
			}
			else if (command.cmd == "-VERIFY")
			{
				string msg = "PornoDB Manager: Verify DB with disk at: ";
				msg += GetTimeStamp();
				DebugPrint(msg.c_str());
				dbBuilder.VerifyDB(command.data[0]);
			}
			else if (command.cmd == "-ADDIMG")
			{
				string galleryPath = MyFileDirDll::getPathFromFullyQualifiedPathString(command.data[0]);
				//argh! sometimes i have a trailing slash at the end of the pathi n theDB, sometimes i dont!
				//galleryPath += "\\";
				int galleryID = WinToDBMiddleman::GetGalleryIDFromPath(galleryPath);

				//couldnt find the gallery
				if (galleryID == -1)
					continue;

				dbBuilder.RequestImageHash(command.data[0], galleryID, &conn, createImageHashSocket);
				
			}
			else if (command.cmd == "-DELIMG")
			{
				string output;
				WinToDBMiddleman::DeleteImageFromDB(command.data[0], output);
			}
			else if (command.cmd == "-MOVIMG")
			{

			}
			else if (command.cmd == "HASH")
			{
				if (!dbBuilder.AddHashDataToDB(command.data[0], command.data[1], command.data[2]))
					printf("error adding: %s\n", command.data[0].c_str());
			}
		}

		g_notified = false;
	}

	ShutdownCreateImageHash();
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