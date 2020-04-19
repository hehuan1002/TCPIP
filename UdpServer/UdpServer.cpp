// UdpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <winsock2.h>
#include <ws2ipdef.h>
#include <iostream>
#pragma comment(lib, "ws2_32.lib")


int main()
{
	int retVal;   
	char buf[BUFSIZ];      //��������С
	

	//��ʼ��socket
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
	{
		printf("WSAStartup �޷���ʼ����");
		return 0;
	}
	
	// �������ڼ�����Socket
	SOCKET sServer;
	sServer = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == sServer)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}

	// ���÷�����Socket��ַ
	SOCKADDR_IN addrServ;
	addrServ.sin_family = AF_INET;
	addrServ.sin_port = htons(27015);
	addrServ.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	retVal = bind(sServer, (const struct sockaddr*)&addrServ, sizeof(addrServ));
	if (SOCKET_ERROR == retVal)
	{
		printf("bind failed! \n");
		closesocket(sServer);
		WSACleanup();
		return -1;
	}

	//
	sockaddr_in senderAddr;  //�����ߴ�С
	int SenderAddrSize = sizeof(senderAddr);
	while (true)
	{
		ZeroMemory(buf, BUFSIZ);
		retVal = recvfrom(sServer,
			buf,
			BUFSIZ,
			0,
			(sockaddr*)&senderAddr,
			&SenderAddrSize);
		if (retVal == SOCKET_ERROR)
		{
			closesocket(sServer);
			WSACleanup();
			break;
		}
		printf("Recv From Client: %s\n", buf);

		if (strcmp(buf, "quit") == 0)
		{
			closesocket(sServer);
			WSACleanup();
			break;
		}

		// �������ݵ��ͻ���
		char msg[BUFSIZ];
		sprintf_s(msg, "Message received - %s", buf);
		retVal = sendto(sServer,
			msg,
			BUFSIZ,
			0,
			(sockaddr*)&senderAddr,
			SenderAddrSize);
		if (retVal == SOCKET_ERROR)
		{
			closesocket(sServer);
			WSACleanup();
			break;
		}

		printf("Send to Client: %s\n", msg);
	}
	

	closesocket(sServer);
	printf("Exiting.\n");
	
    return 0;
}

