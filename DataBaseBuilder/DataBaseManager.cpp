// DataBaseBuilder.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <algorithm>
#include <string>
#include <windows.h>

#include <stdio.h>

#include <vector>

#include "DataBaseManager.h"
#include "DataBaseBuilder.h"

#include "CommandLineUtils.h"
#include "CFGUtils.h"
#include "CFGHelper.h"
#include "WinToDBMiddleman.h"


#include "LogMessage.h"
#include "ConsoleOutput.h"
#include "LogfileOutput.h"

#define SHORT_SLEEP 1
#define LONG_SLEEP 10000
//sc create "PornoDB Manager" binPath= C:\SampleService.exe

void Start()
{
	//START STUFF
	//Sleep(10000);
	logRouter.Log(LOG_LEVEL_INFORMATIONAL,"starting server");
	//conn.StartServer(/*SOMAXCONN,*/ CFGHelper::DataBaseManagerPort.c_str());

	 networkThread = NULL;
	 dirWatchThread = new thread(doDirWatch);
	 waitForClientThread = new thread(doWaitForRemoteConnections);
	 mainThread = new thread(doMainWorkerThread);
}

void Finish()
{
	//conn.Shutdown();
	CancelIo(hDir);
	
	//int exitCode = 0;
	//TerminateThread(dirWatchThread, exitCode);
	//CancelIoEx(hDir, overlapped);
}


LogOutput * LogOutputFactory(string logType, string loggerDetails)
{
	if (logType.compare(CONSOLE_OUTPUT_STRING) == 0)
		return new ConsoleOutput();

	else if (logType.compare(FILE_OUTPUT_STRING) == 0)
		return new LogFileOutput(loggerDetails);

	/*else if (logType.compare(TCP_OUTPUT_STRING) == 0)
		return new TCPOutput();

	else if (logType.compare(EVENT_LOG_OUTPUT_STRING) == 0)
		return new WindowsEventLogOutput();*/


	//uf we get here, we got an invalid type.
	//instad of returning null, return a console output
	return new ConsoleOutput();
}

void GetLoggingInfoFromCFG()
{
	//logRouter.ClearLogEntities();


	vector<string> loggerList = CFGUtils::GetCfgListValue("LOGGERS");

	for (size_t i = 0; i < loggerList.size(); i++)
	{
		string listName = loggerList[i];
		vector<string> curLogger = CFGUtils::GetCfgListValue(listName);

		string loggingType = CFGUtils::GetStringValueFromList(listName, "TYPE");//consle, logfile syslog etc
		string severityLevelString = CFGUtils::GetStringValueFromList(listName, "SEVERITY"); //should have all the logging level values this appender wants
		string loggerDetails = CFGUtils::GetStringValueFromList(listName, "PATH");

		LogOutput * newLogger = LogOutputFactory(loggingType, loggerDetails); //optional, so far, only textfiles have this data
		
		LogEntity * temp = logRouter.AddLogger(newLogger, severityLevelString);
		if (CFGUtils::GetCfgBoolValue("ADD_TIMESTAMP"))
			temp->logOut->useTimeStamp = true;
		if (CFGUtils::GetCfgBoolValue("ADD_SEVERITY"))
			temp->logOut->useSeverityString = true;
	}

}

int main(int argc, char *argv[])
{
	CFGHelper::filePathBase = CFGHelper::SetProgramPath(argv[0]);
	CFGHelper::LoadCFGFile();

	string err;
	if (!CFGHelper::IsCFGComplete(err))
	{
		string msg = "CFG file incomplete: " + err;
		MessageBox(NULL, msg.c_str(), NULL, NULL);
		exit(0);
	}
	
	GetLoggingInfoFromCFG();

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: Main: Entry");

	if(argc >= 2)
		isService = false;

	//when debugging, this is a normal console exe
	if (!isService)
	{
		// Start the thread that will perform the main task of the service
		Start();

		logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Waiting for Worker Thread to complete");


		// Wait until our main worker thread exits effectively signaling that the service needs to stop
		//WaitForSingleObject(hMainThread, INFINITE);
		mainThread->join();
		logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Worker Thread Stop Event signaled");

		/*
		* Perform any cleanup tasks
		*/
		logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Performing Cleanup Operations");

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
		logRouter.Log(LOG_LEVEL_ERROR, errorText.c_str());
		return -1;
	}

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: Main: Exit");
	return 0;
}


VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;
	
	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Entry");

	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceMain: RegisterServiceCtrlHandler returned error");
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
		logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceMain: SetServiceStatus returned error");
	}

	
	

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Performing Service Start Operations");

	// Create stop event to wait on later.
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceMain: SetServiceStatus returned error");
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
		logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceMain: SetServiceStatus returned error");
	}

	// Start the thread that will perform the main task of the service
	Start();

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Waiting for Worker Thread to complete");

	// Wait until our main worker thread exits effectively signaling that the service needs to stop
	//WaitForSingleObject(hMainThread, INFINITE);
	mainThread->join();

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Worker Thread Stop Event signaled");

	/*
	* Perform any cleanup tasks
	*/
	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Performing Cleanup Operations");

	Finish();

	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceMain: SetServiceStatus returned error");
	}

EXIT:
	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceMain: Exit");

	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceCtrlHandler: Entry");

	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			logRouter.Log(LOG_LEVEL_ERROR, "PornoDB Manager: ServiceCtrlHandler: SetServiceStatus returned error");
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

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceCtrlHandler: Exit");
}



//http://qualapps.blogspot.com/2010/05/understanding-readdirectorychangesw.html
int doDirWatch(void)
{
	
	int nCounter = 0;
	FILE_NOTIFY_INFORMATION strFileNotifyInfo[1024];
	DWORD dwBytesReturned = 0;
		
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
		string errString = "fileWatcher invalid handle error: " + to_string(GetLastError());
		logRouter.Log(LOG_LEVEL_ERROR, errString);
		//exit(-1);
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
			string cmdNamePostFix;
			if(isFile)
				cmdNamePostFix = "IMG";
			else
				cmdNamePostFix = "GAL";

			switch (strFileNotifyInfo[0].Action)
			{
				
			case FILE_ACTION_ADDED: //0x00000001
			case FILE_ACTION_RENAMED_NEW_NAME://0x00000005
				//The file was added to the directory.
					newCommand.SetCommand("-ADD"+ cmdNamePostFix);
				break;
			case FILE_ACTION_REMOVED: //0x00000002
			case FILE_ACTION_RENAMED_OLD_NAME://0x00000004
				//The file was removed from the directory.
					newCommand.SetCommand("-DEL"+ cmdNamePostFix);
				break;
			case FILE_ACTION_MODIFIED://0x00000003
				//The file was modified. This can be a change in the time stamp or attributes.
					newCommand.SetCommand("-VERIFY"+ cmdNamePostFix);
				break;
			}

			newCommand.data.push_back(fullPath);
			newCommand.dest = -1;
			delete fileName;
			fileName = NULL;

		//	unique_lock<mutex> locker(g_lockqueue);
			tasks.push(newCommand);
			g_notified = true;
			g_queuecheck.notify_one();
		}
		else
		{
			string errString = "fileWatcher error: " + to_string(GetLastError());
			logRouter.Log(LOG_LEVEL_ERROR, errString);
		}
	}
	return 0;
}
int doWaitForRemoteConnections(void)
{
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		//conn.WaitForFirstClientConnect();
		//if (conn.GetNumConnections() > 0)
		{
			g_hasConnections = true;
			if(networkThread == NULL)
				networkThread = new thread(doNetWorkCommunication);
		}
	}	

	return 0;
}
int doNetWorkCommunication(void)
{
	int iResult;
	int DEFAULT_BUFLEN = 512;
	char recvbuf[512];
	int recvbuflen = DEFAULT_BUFLEN;
	int numConn = 0;
	bool recvData = false;
	clock_t  heartBeat = 0;
	
//	if (conn == NULL)
//		conn = new TCPOutput(/*&conn,0*/);

	while (g_hasConnections)
	{
		recvData = false;
		//numConn = (int)conn.GetNumConnections();
		if (numConn == 0)
		{
			g_hasConnections = false;
			break;
		}
		for (int i = 0; i < numConn; i++)
		{
		//	if (conn.HasRecivedData(i))
			{
				recvData = true;
				//iResult = conn.GetData(i, recvbuf, DEFAULT_BUFLEN);
				if (iResult > 0)
				{
					if (iResult < DEFAULT_BUFLEN)
						recvbuf[iResult] = '\0';
					//printf("%s -> %d bytes.\n", recvbuf, iResult);

					vector<string> argVec = StringUtils::Tokenize(recvbuf, "|");
					CmdArg newCommand = parseCommand(argVec);

					newCommand.dest = i;
						
					//unique_lock<mutex> locker(g_lockqueue);
					tasks.push(newCommand);
					g_notified = true;
					g_queuecheck.notify_one();

					if(i == createImageHashSocket)
						heartBeat = clock();
				}
				//client disconnected
				else if (iResult == 0)
				{
					//conn.CloseConnection(i);
				}
				else
				{
					string errString = "network communication error: " + to_string(GetLastError());
					logRouter.Log(LOG_LEVEL_ERROR, errString);
					//conn.CloseConnection(i);
				}
			}
		}
		if (!recvData)
		{
			Sleep(SHORT_SLEEP);  
			float curTime =(float) (clock() - heartBeat) / CLOCKS_PER_SEC;
			if(curTime > 60000)
				ShutdownCreateImageHash();
		}
	}

	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: network communication: Exit");
	networkThread = NULL;
//	delete tcpOut;
//	tcpOut = NULL;
	return ERROR_SUCCESS;
}

int doMainWorkerThread(void)
{
	DatabaseBuilder dbBuilder(CFGHelper::dbPath, CFGHelper::ignorePattern);
	dbBuilder.FillMetaWords(CFGHelper::meta);
	dbBuilder.SetLogRouter(&logRouter);

	vector<string> dbTableValues = CFGUtils::GetCfgListValue("tableNames");
	//if we cant find the table names in the cfg, thejust get out of here
	if (dbTableValues.size() == 1 && dbTableValues[0].find("could not find") != string::npos)
	{
		cout << "couldnt find the list of table names in your cfg...\n";
		return -1;
	}

	dbBuilder.FillPartsOfSpeechTable(dbTableValues);
	//dbBuilder.verifyDB(CFGHelper::pathToProcess);
	
	logRouter.Log(LOG_LEVEL_INFORMATIONAL, "PornoDB Manager: ServiceWorkerThread: Entry");

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
				//temp code!
				//tcpOut->ChangeSocketIndex(command.dest);
				//dbBuilder.logger = tcpOut;

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
				SendReportBackOriginator(report, command.dest);
			}
			else if (command.cmd == "-DELGAL")
			{
				string output;
				WinToDBMiddleman::DeleteGalleryFromDB(command.data[0], output);
				if(output.empty())
					output = command.data[0] + "delete successful";
				SendReportBackOriginator(output, command.dest);
			}
			else if (command.cmd == "-MOVGAL")
			{
				WinToDBMiddleman::MoveGalleryOnDB(command.data[0], command.data[1]);
				SendReportBackOriginator("did you really want to move a gallery?", command.dest);
			}
			else if (command.cmd == "-VERIFYGAL")
			{
				
				string msg = "PornoDB Manager: Verify DB with " + command.data[0] +" START: ";
				SendReportBackOriginator(msg, command.dest);

				dbBuilder.VerifyDB(command.data[0]);

				msg = "PornoDB Manager: Verify DB with " + command.data[0] + " FINISH: ";
				SendReportBackOriginator(msg, command.dest);
			}
			else if (command.cmd == "-VERIFYIMG")
			{
				int galleryID = -1;
				if (!dbBuilder.VerifyImage(command.data[0], galleryID))
					dbBuilder.RequestImageHash(command.data[0], galleryID, /*&conn,*/ createImageHashSocket);
			}
			else if (command.cmd == "-ADDIMG")
			{
				string galleryPath = FileUtils::GetPathFromFullyQualifiedPathString(command.data[0]);
				//argh! sometimes i have a trailing slash at the end of the pathi n theDB, sometimes i dont!
				//galleryPath += "\\";
				int galleryID = WinToDBMiddleman::GetGalleryIDFromPath(galleryPath);

				//couldnt find the gallery
				if (galleryID == -1)
					continue;

				dbBuilder.RequestImageHash(command.data[0], galleryID, /*&conn,*/ createImageHashSocket);
				
			}
			else if (command.cmd == "-DELIMG")
			{
				string output;
				WinToDBMiddleman::DeleteImageFromDB(command.data[0], output);
				if (output.empty())
					output = command.data[0] + "delete successful";
				SendReportBackOriginator(output, command.dest);
			}
			else if (command.cmd == "-MOVIMG")
			{
				WinToDBMiddleman::MoveImageOnDB(command.data[0], command.data[1]);
				SendReportBackOriginator("did you really want to move an image?", command.dest);
			}
			else if (command.cmd == "-HASH")
			{
				if (!dbBuilder.AddHashDataToDB(command.data[0], command.data[1], command.data[2]))
					logRouter.Log(LOG_LEVEL_ERROR, "error adding hash: "+ command.data[0]);
			}
			else
			{
				string msg = "unrecognized command: " + command.cmd;
				logRouter.Log(LOG_LEVEL_WARNING, msg);

				if(command.dest != createImageHashSocket)
					SendReportBackOriginator(msg, command.dest);
			}
		}

		g_notified = false;
	}

	ShutdownCreateImageHash();
	return 0;
}