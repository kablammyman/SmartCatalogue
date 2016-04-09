#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string>
#include <queue>
#include "Utils.h"

#include "CreateImageHash.h"

int main(int argc, const char *argv[])
{
	bool done = true;
	bool err = false;
	string output = "";
	char recvbuf[DEFAULT_BUFLEN];

	vector<string> argVec(argv,argv + argc);
	HashCmdArg newCommand = parseCommand(argVec);
	newCommand.dest = -1;
	queue<HashCmdArg> allArgs;
	allArgs.push(newCommand);

	if (isServer)
	{
		cout << "starting server\n";
		conn.startServer(SOMAXCONN, port);
		done = false;
	}
	
	do
	{
		if (isServer)
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
						argVec.clear();
						argVec = Utils::tokenize(recvbuf, ",");
						HashCmdArg newCommand = parseCommand(argVec);
						newCommand.dest = i;
						allArgs.push(newCommand);
					}
					//client disconnected
					else if (iResult == 0)
						conn.closeConnection(i);

					else
					{
						printf("recv failed: %d\n", WSAGetLastError());
						done = true;
					}
				}
			}
		}

		if (!allArgs.empty())
		{
			HashCmdArg curCommand = allArgs.front();
			allArgs.pop();

			if (curCommand.shutdown)
			{
				done = true;
				break;
			}

			output = ExecuteCommand(curCommand);
			
			if (output.empty())
				output = "didnt recognize any params...";

			if (curCommand.dest == -1)
				cout << output << std::flush;
			else if (curCommand.dest > -1)
				conn.sendData(curCommand.dest, output.c_str());
		}

	} while (!done);

	
	if(isServer)
		conn.shutdown();

	if(!err)
		return 0;
	return -1;
}