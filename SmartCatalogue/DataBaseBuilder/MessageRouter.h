#pragma once

#include <Windows.h>
#include <string>
#include <fstream>
#include "NetworkConnection.h"
using namespace std;

#define LOG_LEVEL_DEBUG 0
#define DEBUG_PREFIX "DEBUG"
#define LOG_LEVEL_INFORMATIONAL 1
#define INFORMATIONAL_PREFIX "INFORMATIONAL"
#define LOG_LEVEL_NOTICE 2
#define NOTICE_PREFIX "NOTICE"
#define LOG_LEVEL_WARNING 3
#define WARNING_PREFIX "WARNING"
#define LOG_LEVEL_ERROR 4
#define ERROR_PREFIX "ERROR"
#define LOG_LEVEL_CRITICAL 5
#define CRITICAL_PREFIX "CRITICAL"
#define LOG_LEVEL_ALERT 6
#define ALERT_PREFIX "ALERT"
#define LOG_LEVEL_EMERGENCY 7
#define EMERGENCY_PREFIX "EMERGENCY"

	
//so we know where to send strings like error message
class LogOutput
{
protected:

	string GetStringFromSeverity(int severity);
	int GetValueFromSeverityString(string severity);
	string GetTimeStamp();
public:
	virtual void WriteMessage(int severity, string message) = 0;
};

class ConsoleOutput : public LogOutput
{
public:
	//ConsoleOutput();//create a new console window for output
	//~ConsoleOutput();
	void WriteMessage(int severity, string message);
};

class LogFileOutput : public LogOutput
{
	fstream fs;
public:
	LogFileOutput(string testFile)
	{
		fs.open(testFile, fstream::in | fstream::out | fstream::app);
	}
	
	~LogFileOutput()
	{
		fs.close();
	}

	void WriteMessage(int severity, string message);
};

class WindowsEventLogOutput : public LogOutput
{
	WORD ConvertSeverityToWindows(int severity);
public:
	void WriteMessage(int severity, string message);
};

class TCPOutput : public LogOutput
{
	NetworkConnection *conn;
	int socket;
public:
	TCPOutput(NetworkConnection *c, int s)
	{
		conn = c;
		socket = s;
	}
	void ChangeSocketIndex(int index)
	{
		if(index > -1)
			socket = index;
	}
	void WriteMessage(int severity, string message);
};