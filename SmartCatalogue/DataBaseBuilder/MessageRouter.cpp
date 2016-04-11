#pragma once


#include "MessageRouter.h"
#include <ctime>//check execution time
#include <iostream>

int LogOutput::GetValueFromSeverityString(string severity)
{

	//wstringToupper(severity);

	if (severity.compare(DEBUG_PREFIX) == 0)
		return LOG_LEVEL_DEBUG;

	else if (severity.compare(INFORMATIONAL_PREFIX) == 0)
		return LOG_LEVEL_INFORMATIONAL;

	else if (severity.compare(NOTICE_PREFIX) == 0)
		return LOG_LEVEL_NOTICE;

	else if (severity.compare(WARNING_PREFIX) == 0)
		return LOG_LEVEL_WARNING;

	else if (severity.compare(ERROR_PREFIX) == 0)
		return LOG_LEVEL_ERROR;

	else if (severity.compare(EMERGENCY_PREFIX) == 0)
		return LOG_LEVEL_EMERGENCY;

	else if (severity.compare(CRITICAL_PREFIX) == 0)
		return LOG_LEVEL_CRITICAL;

	else if (severity.compare(ALERT_PREFIX) == 0)
		return LOG_LEVEL_ALERT;


	//if we get here, then we dont have a severity level keyword
	return 0;
}

//-------------------------------------------------------------------------------------
string LogOutput::GetStringFromSeverity(int severity)
{
	switch (severity)
	{
	case LOG_LEVEL_DEBUG:
		return DEBUG_PREFIX;
	case LOG_LEVEL_INFORMATIONAL:
		return INFORMATIONAL_PREFIX;
	case LOG_LEVEL_NOTICE:
		return NOTICE_PREFIX;
	case LOG_LEVEL_WARNING:
		return WARNING_PREFIX;
	case LOG_LEVEL_ERROR:
		return ERROR_PREFIX;
	case LOG_LEVEL_CRITICAL:
		return CRITICAL_PREFIX;
	case LOG_LEVEL_ALERT:
		return ALERT_PREFIX;
	case LOG_LEVEL_EMERGENCY:
		return EMERGENCY_PREFIX;
	}
	return DEBUG_PREFIX;
}
//-------------------------------------------------------------------------------------
string LogOutput::GetTimeStamp()
{
	time_t rawtime;
	struct tm * timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	return asctime(timeinfo);
	//printf("The current date/time is: %s", asctime(timeinfo));
}


void ConsoleOutput::WriteMessage(int severity,  string message)
{
	cout << GetTimeStamp() << " " << GetStringFromSeverity(severity) <<": " << message <<endl;
}

void LogFileOutput::WriteMessage(int severity, string message)
{
	fs << GetTimeStamp() << " " << GetStringFromSeverity(severity) << ": " << message << endl;
}

WORD WindowsEventLogOutput::ConvertSeverityToWindows(int severity)
{
	switch (severity)
	{
	case LOG_LEVEL_INFORMATIONAL:
	case LOG_LEVEL_NOTICE:
		return EVENTLOG_INFORMATION_TYPE;

	case LOG_LEVEL_WARNING:
		return EVENTLOG_WARNING_TYPE;

	case LOG_LEVEL_ERROR:
	case LOG_LEVEL_CRITICAL:
	case LOG_LEVEL_ALERT:
	case LOG_LEVEL_EMERGENCY:
		return EVENTLOG_ERROR_TYPE;
	default:
		return EVENTLOG_INFORMATION_TYPE;
	}
}


void WindowsEventLogOutput::WriteMessage(int severity,   string message)
{
	char temp[1023] = { 0 };
	WORD wType = ConvertSeverityToWindows(severity);

	HANDLE hEventSource;
	LPSTR lpszStrings[1];

	if (message.length() > _countof(temp)) {
		message = message.substr(0, _countof(temp) - 10);
		message += "...";
	}


	sprintf_s(temp, _countof(temp),"%s", message.c_str());
	lpszStrings[0] = temp;

	// Get a handle to use with ReportEvent().
	//hEventSource = RegisterEventSource(NULL, sSourceName.c_str());
	hEventSource = RegisterEventSource(NULL, "SmartCatalogue");
	if (hEventSource != NULL)
	{
		// Write to event log.
		ReportEvent(
			hEventSource,						//event source
			wType,								//event type
			0,									//category - none
			0x00000001L,						//event id
			NULL,								//used Sid
			1,									//num strings
			0,									//data size
			(LPCTSTR*)&lpszStrings[0],			//strings
			NULL);								//raw data
		DeregisterEventSource(hEventSource);
	}

	return;
}

void TCPOutput::WriteMessage(int severity,  string message)
{
	string msg = (GetTimeStamp() + " " + GetStringFromSeverity(severity) + ": " + message + "\n");
	conn->sendData(socket, msg.c_str());
}
