// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 64   // 缓冲区大小

int main()
{
	SOCKET sServer;
	SOCKET sClient;
	int retVal;         // 调用各种Socket函数的返回值
	char buf[BUF_SIZE]; // 用于接收客户端数据的缓冲区

						// 初始化Socket动态库
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup 无法初始化！");
		return 0;
	}

	// 创建用于监听的Socket
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sServer)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}

	// 设置服务器Socket地址
	SOCKADDR_IN addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(9990);
	addrServ.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// 绑定Socket Server到本地地址
	retVal = bind(sServer, (const struct sockaddr*)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		printf("bind failed! \n");
		closesocket(sServer);
		WSACleanup();
		return -1;
	}


	//在Socket Server上进行监听
	retVal = listen(sServer, 1);
	if (SOCKET_ERROR == retVal)
	{
		printf("listen failed!\n");
		closesocket(sServer);
		WSACleanup();
		return -1;
	}

	//接受来自客户端的请求
	printf("TCP Server start...\n");
	SOCKADDR_IN addrClient;
	//ZeroMemory(&addrClient, sizeof(addrClient));
	int addrClientLen = sizeof(addrClient);
	sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientLen);
	if (INVALID_SOCKET == sClient)
	{
		printf("accept failed!\n");
		closesocket(sServer);
		WSACleanup();
		return -1;
	}

	// 循环接受客户端的数据，直到客户端发送quit命令后推出。
	while (true)
	{
		ZeroMemory(buf, BUF_SIZE); // 清空接受数据的缓冲区
		retVal = recv(sClient, buf, BUF_SIZE, 0);
		char tempBuf[BUF_SIZE]; 
		memset(tempBuf, 0, sizeof(tempBuf));
		int ttl = 0;
		getsockopt(sClient, IPPROTO_TCP, TCP_MAXSEG, tempBuf, (int*)&ttl);
		if (SOCKET_ERROR == retVal)
		{
			printf("recv failed!\n");
			closesocket(sServer);
			closesocket(sClient);
			WSACleanup();
			return -1;
		}

		// 获取当前系统时间
		SYSTEMTIME st;
		GetLocalTime(&st);
		char sDateTime[30];
		sprintf_s(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		printf("%s, Recv From Client [%s-%d]:%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), addrClient.sin_port, buf);

		if (strcmp(buf, "quit") == 0)
		{
			retVal = send(sClient, "quit", strlen("quit"), 0);

			// 释放Socket
			closesocket(sServer);
			closesocket(sClient);
			WSACleanup();

			break;
		}
		else
		{
			char msg[BUF_SIZE];
			sprintf_s(msg, "Message received - %s", buf);
			retVal = send(sClient, msg, strlen(msg), 0);
			if (SOCKET_ERROR == retVal)
			{
				printf("send failed!\n");
				closesocket(sServer);
				closesocket(sClient);
				WSACleanup();
				return -1;
			}
		}
	}
	return 0;
}