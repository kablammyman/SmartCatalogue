#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <string>

#include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"

#include "CreateImageHash.h"

#include "myFileDirDll.h"
#include "similarImage.h"
#include "Database.h"


struct Options
{
	int dist;
	string dbPath;
	string pathToProcess;

	Options()
	{
		dist = 2;
		dbPath = "";
		pathToProcess = "";
	}
};
struct CmdArg
{
	string cmd;
	vector<string> data;
	//what will the optiosn be when executing this command
	Options opts;
};

int main(int argc, const char *argv[])
{
	//START STUFF
	cout << "starting server\n";
	conn.startServer(SOMAXCONN, port);
	bool done = false;

	if (argc < 3)
		invalidParmMessageAndExit();

	int i = 0;
	SimilarImage simImage;
	string output = "";
	bool err = false;
	string cmdArgUpper;
	
	vector<CmdArg> allArgs;

	//how to distinguish between commandline args vs socket args?
	//how to make all go thry same pipeline?

	while (i < argc)
	{
		string temp = argv[i];
		cmdArg newArg;

		if (temp[0] == '-')
		{
			newArg.flag = temp[0];
			i++;
			while (i < argc)
			{
				if(argv[i][0] != '-')
					newArg.data.push_back(argv[i]);
				i++;
			}
		}

		
	}
	while (!done)
	{
		numConn = (int)conn.getNumConnections();
		for (int i = 0; i < numConn; i++)
		{
			if (conn.hasRecivedData(i))
			{
				iResult = conn.getData(i, recvbuf, DEFAULT_BUFLEN);
				if (iResult > 0)
				{
					recvbuf[iResult] = '\0';
					printf("%s -> %d bytes.\n", recvbuf, iResult);

					switch (parseCommand(recvbuf))
					{
					case 0:
						time_t rawtime;
						struct tm * timeinfo;

						time(&rawtime);
						timeinfo = localtime(&rawtime);
						//printf("The current date/time is: %s", asctime(timeinfo));
						conn.sendData(i, asctime(timeinfo));
						break;
					case 1:
						conn.sendData(i, "Fred");
						break;
					default:
						conn.sendData(i, "unknown command");
						break;
					}

				}
				//client disconnected
				else if (iResult == 0)
					conn.closeConnection(i);

				else
					printf("recv failed: %d\n", WSAGetLastError());
			}
		}
	}


	if (output.empty())
		output = "didnt recognize any params...";
	
	cout << output << std::flush;
	//cout << endl; //when other programs read from this programs stdout, we dont want the new line with the actual data
	
	conn.shutdown();

	if(!err)
		return 0;
	return -1;
}