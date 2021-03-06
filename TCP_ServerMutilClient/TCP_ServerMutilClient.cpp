// TCP_ServerMutilClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>

#pragma comment (lib, "ws2_32.lib")

using namespace std;

void main()
{
	//Step 1: Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return;
	}
	//Step 2: Construct a socket
	SOCKET listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listenSock == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return;
	}

	//Step 3: Bind the ip address and port to a socket
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	InetPton(AF_INET, _T("127.0.0.1"), &serverAddr.sin_addr.s_addr);
	if (bind(listenSock, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		cout << "Error! Cannot bind this addresss. ";
		system("pause");
		return;
	}
	//Step 4: Listening request form client 
	listen(listenSock, 10);
	cout << "Waiting client connect...\n";
	// Create the master file descriptor set and set it is zero
	fd_set master;
	FD_ZERO(&master);

	// Add listenSock to the fist socket in file descriptor
	FD_SET(listenSock, &master);

	//Step 5: Comunicate with client
	bool running = true;

	while (running)
	{
		// Make a copy of the master file descriptor
		fd_set copy = master;

		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listenSock)
			{
				// Accept a new connection
				SOCKET client = accept(listenSock, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);
				cout << "Socket #" << client << " conected. \n";

				// Send a welcome message to the connected client
				string welcomeMsg = "Welcome to the Awesome Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);

				// Send a warring message to other client
				for (int i = 0; i < master.fd_count; i++)
				{
					SOCKET outSock = master.fd_array[i];

					// Don't send massge to linstenSock and Sock.
					if (outSock != listenSock && outSock != sock)
					{
						ostringstream ss;
						ss << "SOCKET #" << client << " just conected "<< "\r\n";
						string strOut = ss.str();
						send(outSock, strOut.c_str(), strOut.size() + 1, 0);
					}
				}
			}

			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);
				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);

				if (bytesIn <= 0) {
					closesocket(sock);
					FD_CLR(sock, &copy);
				}
				else
				{
					// Check to see if it's a command "\exit" kills the server
					if (buf[0] == '\\')
					{
						// If it is exit message.
						string cmd = string(buf, bytesIn);
						if (cmd == "\\exit")
						{
							running = false;
							break;
						}
						// Unknown command
						continue;
					}

					// Send message to other clients
					for (int i = 0; i < master.fd_count; i++)
					{
						SOCKET outSock = master.fd_array[i];

						// Don't send massge to linstenSock and Sock.
						if (outSock != listenSock && outSock != sock)
						{
							ostringstream ss;
							ss << "SOCKET #" << sock << ": " << buf << "\r\n";
							string strOut = ss.str();
							send(outSock, strOut.c_str(), strOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(listenSock, &master);
	closesocket(listenSock);

	// Message to let users know what's happening.
	string msg = "Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		SOCKET sock = master.fd_array[0];
		send(sock, msg.c_str(), msg.size() + 1, 0);
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	//Step 6: Cleanup winsock
	WSACleanup();

	system("pause");
}

