// View_Client.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"
#include "stdio.h"
#include"WinSock2.h"
#include "ws2tcpip.h"
#pragma comment (lib, "ws2_32.lib")

int main()
{
	//Step 1: Innitiate WinSock
	WORD wVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(wVersion, &wsaData))
		printf("Can not start this version! \n");
	//Step 2: Construct Socket
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

	//(Optional) Set time-out for receiving
	int tv = 1000000;// Time-out interval: 10000ms
	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));

	//Step 3: Spetify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5500);
	InetPton(AF_INET, _T("127.0.0.1"), &serverAddr.sin_addr.s_addr);
	//Step 4; Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr)))
	{
		printf("Error ! Cannot connect server. \n");
		system("pause");
		return 0;
	}
	//Step 5: Communicate with server
	char buff[1024];
	int ret;
	do {
		//Receive message
		ret = recv(client, buff, 1024, 0);
		if (ret == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAETIMEDOUT)
				printf("Time out !");
			else printf("Error! cannot reveive message. ");
		}
		else
		{
			buff[ret] = '\0';
			printf("%s\n", buff);
		}
	} while (true);
	//Step 6: Cloese socket
	shutdown(client, SD_SEND);
	//Step 7: Terminate Winsock
	WSACleanup();
	system("pause");
	return 0;
}



