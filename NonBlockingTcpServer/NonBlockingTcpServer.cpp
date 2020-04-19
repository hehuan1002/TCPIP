// NonBlockingTcpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")
#define BUF_SIZE 64   // ��������С

DWORD WINAPI AnswerThread(LPVOID lparam)
{
	char buf[BUF_SIZE]; // ���ڽ��տͻ������ݵĻ�����
	int retVal;
	// ������lparamת��Ϊ�ͻ��˽���ͨ�ŵ�Socket���

}



int main()
{
	SOCKET sServer;
	SOCKET sClient;
	int retVal;         // ���ø���Socket�����ķ���ֵ

	// ��ʼ��Socket��̬��					
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup �޷���ʼ����");
		return 0;
	}

	// �������ڼ�����Socket
	sServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (INVALID_SOCKET == sServer)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}

	// ����SocketΪ������ģʽ
	int iMode = 1;
	retVal = ioctlsocket(sServer, FIONBIO, (u_long FAR*)&iMode);
	if (retVal == SOCKET_ERROR)
	{
		printf("ioctlsocket failed!\n");
		WSACleanup();
		return -1;
	}

	// ���÷�����Socket��ַ
	SOCKADDR_IN addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(9990);
	addrServ.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

	// ��Socket Server�����ص�ַ
	retVal = bind(sServer, (const struct sockaddr*)&addrServ, sizeof(SOCKADDR_IN));
	if (SOCKET_ERROR == retVal)
	{
		printf("bind failed! \n");
		closesocket(sServer);
		WSACleanup();
		return -1;
	}


	//��Socket Server�Ͻ��м���
	retVal = listen(sServer, 1);
	if (SOCKET_ERROR == retVal)
	{
		printf("listen failed!\n");
		closesocket(sServer);
		WSACleanup();
		return -1;
	}

	//�������Կͻ��˵�����
	printf("TCP Server start...\n");
	SOCKADDR_IN addrClient;
	int addrClientLen = sizeof(addrClient);
	while (TRUE)
	{
		sClient = accept(sServer, (sockaddr FAR*)&addrClient, &addrClientLen);
		if (INVALID_SOCKET == sClient)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK) // �޷�������ɷ�����Socket�ϵĲ���
			{
				Sleep(100);
				continue;
			}
			else
			{
				printf("accept failed!\n");
				closesocket(sServer);
				WSACleanup();
				return -1;
			}
		}
		DWORD dwThreadId;
		CreateThread(NULL, NULL, AnswerThread, (LPVOID)sClient, 0, &dwThreadId);
	}
	

	// ѭ�����ܿͻ��˵����ݣ�ֱ���ͻ��˷���quit������Ƴ���
	while (true)
	{
		ZeroMemory(buf, BUF_SIZE); // ��ս������ݵĻ�����
		retVal = recv(sClient, buf, BUF_SIZE, 0);
		if (SOCKET_ERROR == retVal)
		{
			int err = WSAGetLastError();
			if (err == WSAEWOULDBLOCK)
			{
				Sleep(100);
				continue;
			}
			else if (err == WSAETIMEDOUT || err == WSAENETDOWN)
			{
				printf("recv failed!\n");
				closesocket(sServer);
				closesocket(sClient);
				WSACleanup();
				return -1;
			}
		}

		// ��ȡ��ǰϵͳʱ��
		SYSTEMTIME st;
		GetLocalTime(&st);
		char sDateTime[30];
		sprintf_s(sDateTime, "%4d-%2d-%2d %2d:%2d:%2d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
		printf("%s, Recv From Client [%s-%d]:%s\n", sDateTime, inet_ntoa(addrClient.sin_addr), addrClient.sin_port, buf);

		if (strcmp(buf, "quit") == 0)
		{
			retVal = send(sClient, "quit", strlen("quit"), 0);
			break;
		}
		else
		{
			char msg[BUF_SIZE];
			sprintf_s(msg, "Message received - %s", buf);
			while (TRUE)
			{
				// ���������������
				retVal = send(sClient, msg, strlen(msg), 0);
				if (SOCKET_ERROR == retVal)
				{
					int err = WSAGetLastError();
					if (err == WSAEWOULDBLOCK)
					{
						Sleep(500);
						continue;
					}
					else
					{
						printf("send failed!\n");
						closesocket(sServer);
						closesocket(sClient);
						WSACleanup();
						return -1;
					}
				}
				break;
			}
			
		}
	}

	return 0;
}
