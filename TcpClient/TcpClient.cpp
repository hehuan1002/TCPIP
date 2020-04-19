// TcpClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <string>
#include <iostream>
#pragma comment(lib, "WS2_32.lib")

int main()
{
	char buf[BUFSIZ];
	SOCKET sHost;  // 与服务器进行通信的Socket
	int retVal;      // 调用各种Socket函数的返回值

	// 初始化Socket动态库
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("WSAStartup failed! \n");
		return -1;
	}

	// 创建Socket
	sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sHost)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}

	// 设置服务器地址
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(9990);
	int sServerAddLen = sizeof(servAddr);

	// 连接服务器
	retVal = connect(sHost, (LPSOCKADDR)&servAddr, sizeof(servAddr));
	if (SOCKET_ERROR == retVal)
	{
		printf("connect failed!\n");
		closesocket(sHost);
		WSACleanup();
		return -1;
	}

	//循环像服务器发送字符串，并显示反馈信息
	// 发送"quit"将使服务器程序退出，同时客户端程序自身也将退出
	while (TRUE)
	{
		printf("Please input a string to send: ");
		std::string str;
		// 接受输入的数据
		std::getline(std::cin, str);
		// 将用户输入的数据复制到buf中
		ZeroMemory(buf, BUFSIZ);
		strcpy(buf, str.c_str());
		//向服务器发送数据
		retVal = send(sHost,buf,strlen(buf),0);
		if (SOCKET_ERROR == retVal)
		{
			printf("send failed !\n");
			closesocket(sHost);
			WSACleanup();
			return -1;
		}

		//接受服务器回传的数据
		retVal = recv(sHost, buf, sizeof(buf)+1, 0);
		printf("Recv From Server: %s\n",buf);

		//如果收到"quit"，则退出
		if (strcmp(buf, "quit") == 0)
		{
			printf("quit!\n");
			closesocket(sHost);
			WSACleanup();
			break;
		}
	}
    return 0;
}

