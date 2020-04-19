// UdpClient.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <Winsock2.h>
#include <string>
#include <iostream>
#pragma comment(lib, "WS2_32.lib")

int main()
{
	char buf[BUFSIZ];
	SOCKET SendSocket;  // �����������ͨ�ŵ�Socket
	int retVal;  

	// ��ʼ��Socket��̬��
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed! \n");
		return -1;
	}

	// ����Socket
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == SendSocket)
	{
		printf("socket failed!\n");
		WSACleanup();
		return -1;
	}

	// ���÷�������ַ
	SOCKADDR_IN servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(27015);
	int sServerAddLen = sizeof(servAddr);

	//��������������ݱ�
	while (TRUE)
	{
		printf("Please input a string to send: ");
		std::string str;
		// �������������
		std::getline(std::cin, str);
		// ���û���������ݸ��Ƶ�buf��
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
		//����յ�"quit"�����˳�
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
	

	//������ɣ��ر�Socket
	printf("Finished sending, Closeing socket\n");
	// �ͷ���Դ���˳�
	printf("Exiting.\n");
	WSACleanup();


    return 0;
}

