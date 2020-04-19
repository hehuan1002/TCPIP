// UdpClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <string>
#include <iostream>
#pragma comment(lib, "WS2_32.lib")

int main()
{
	char buf[BUFSIZ];
	SOCKET SendSocket;  // 与服务器进行通信的Socket
	int retVal;  

	// 初始化Socket动态库
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed! \n");
		return -1;
	}

	// 创建Socket
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == SendSocket)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}

	// 设置服务器地址
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(27015);
	int sServerAddLen = sizeof(servAddr);

	//向服务器发送数据报
	while (TRUE)
	{
		printf("Please input a string to send: ");
		std::string str;
		// 接受输入的数据
		std::getline(std::cin, str);
		// 将用户输入的数据复制到buf中
		ZeroMemory(buf, BUFSIZ);
		strcpy(buf, str.c_str());

		retVal = sendto(SendSocket,
			buf,
			BUFSIZ,
			0,
			(sockaddr*)&servAddr,
			sServerAddLen);
		if (SOCKET_ERROR == retVal)
		{
			closesocket(SendSocket);
			break;
		}
		//如果收到"quit"，则退出
		if (strcmp(buf, "quit") == 0)
		{
			closesocket(SendSocket);
			break;
		}

		retVal = recvfrom(SendSocket,
			buf, 
			BUFSIZ, 
			0, 
			(sockaddr*)&servAddr,
			&sServerAddLen);
		if (SOCKET_ERROR == retVal)
		{
			closesocket(SendSocket);
			break;
		}

		printf("Recv From Server: %s\n", buf);
	}
	

	//发送完成，关闭Socket
	printf("Finished sending, Closeing socket\n");
	// 释放资源，退出
	printf("Exiting.\n");
	WSACleanup();


    return 0;
}

