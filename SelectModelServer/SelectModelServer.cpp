// TcpServer.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <iostream>

#pragma comment(lib,"WS2_32.lib")   

#define   PORT   9990   
#define   DATA_BUFSIZE   8192   

// �����׽�����Ϣ
typedef   struct   _SOCKET_INFORMATION {
	CHAR   Buffer[DATA_BUFSIZE];		// ���ͺͽ������ݵĻ�����
	WSABUF   DataBuf;						// ���巢�ͺͽ������ݻ������Ľṹ�壬�����������ĳ��Ⱥ�����
	SOCKET   Socket;							// ��ͻ��˽���ͨ�ŵ��׽���
	DWORD   BytesSEND;					// �����׽��ַ��͵��ֽ���
	DWORD   BytesRECV;					// �����׽��ֽ��յ��ֽ���
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

DWORD   TotalSockets = 0;				// ��¼����ʹ�õ��׽���������
LPSOCKET_INFORMATION   SocketArray[FD_SETSIZE];			// ����Socket��Ϣ��������飬FD_SETSIZE��ʾSELECTģ�������������׽�������

														// ����SOCKET��Ϣ
BOOL   CreateSocketInformation(SOCKET   s)
{
	LPSOCKET_INFORMATION   SI;										// ���ڱ����׽��ֵ���Ϣ       
																	//   printf("Accepted   socket   number   %d\n",   s);			// ���ѽ��ܵ��׽��ֱ��
																	// ΪSI�����ڴ�ռ�
	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc()   failed   with   error   %d\n", GetLastError());
		return   FALSE;
	}
	// ��ʼ��SI��ֵ    
	SI->Socket = s;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;
	// ��SocketArray����������һ����Ԫ�أ����ڱ���SI���� 
	SocketArray[TotalSockets] = SI;
	TotalSockets++;						// �����׽�������

	return(TRUE);
}

// ������SocketArray��ɾ��ָ����LPSOCKET_INFORMATION����
void   FreeSocketInformation(DWORD   Index)
{
	LPSOCKET_INFORMATION SI = SocketArray[Index];	// ��ȡָ��������Ӧ��LPSOCKET_INFORMATION����
	DWORD   i;
	// �ر��׽���
	closesocket(SI->Socket);
	//printf("Closing   socket   number   %d\n",   SI->Socket);   
	// �ͷ�ָ��LPSOCKET_INFORMATION������Դ
	GlobalFree(SI);
	// ��������index���������Ԫ��ǰ��
	for (i = Index; i < TotalSockets; i++)
	{
		SocketArray[i] = SocketArray[i + 1];
	}
	TotalSockets--;		// �׽���������1
}

// ������������������
int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET   ListenSocket;					// �����׽���
	SOCKET   AcceptSocket;					// ��ͻ��˽���ͨ�ŵ��׽���
	SOCKADDR_IN   InternetAddr;			// �������ĵ�ַ
	WSADATA   wsaData;						// ���ڳ�ʼ���׽��ֻ���
	INT   Ret;											// WinSock API�ķ���ֵ
	FD_SET   WriteSet;							// ��ȡ��д�Ե��׽��ּ���
	FD_SET   ReadSet;							// ��ȡ�ɶ��Ե��׽��ּ���
	DWORD   Total = 0;								// ���ھ���״̬���׽�������
	DWORD   SendBytes;						// ���͵��ֽ���
	DWORD   RecvBytes;						// ���յ��ֽ���


											// ��ʼ��WinSock����
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
		return -1;
	}
	// �������ڼ������׽��� 
	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// ���ü�����ַ�Ͷ˿ں�
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);
	// �󶨼����׽��ֵ����ص�ַ�Ͷ˿�
	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// ��ʼ����
	if (listen(ListenSocket, 5))
	{
		printf("listen()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// ����Ϊ������ģʽ
	ULONG NonBlock = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return -1;
	}
	// ΪListenSocket�׽��ִ�����Ӧ��SOCKET_INFORMATION
	// �����Ϳ��԰�ListenSocket��ӵ�SocketArray������
	CreateSocketInformation(ListenSocket);
	while (TRUE)
	{
		// ׼����������I/O֪ͨ�Ķ�/д�׽��ּ���
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);
		// ��ReadSet��������Ӽ����׽���ListenSocket
		FD_SET(ListenSocket, &ReadSet);
		// ��SocketArray�����е������׽�����ӵ�WriteSet��ReadSet������
		// SocketArray�����б����ż����׽��ֺ�������ͻ��˽���ͨ�ŵ��׽���
		// �����Ϳ���ʹ��select()�ж��ĸ��׽����н������ݻ��߶�ȡ/д������
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
			FD_SET(SocketInfo->Socket, &WriteSet);
			FD_SET(SocketInfo->Socket, &ReadSet);
		}
		// �ж϶�/д�׽��ּ����о������׽���    
		if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
		{
			printf("select()   returned   with   error   %d\n", WSAGetLastError());
			return -1;
		}
		// ���δ��������׽��֡�����������һ����Ӧ�������������ӿͻ����յ����ַ����ٷ��ص��ͻ��ˡ�
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];			// SocketInfoΪ��ǰҪ������׽�����Ϣ
																		// �жϵ�ǰ�׽��ֵĿɶ��ԣ����Ƿ��н��������������߿��Խ�������
			if (FD_ISSET(SocketInfo->Socket, &ReadSet))
			{
				if (SocketInfo->Socket == ListenSocket)		// ���ڼ����׽�����˵���ɶ���ʾ���µ���������
				{
					Total--;	// �������׽��ּ�1
								// �����������󣬵õ���ͻ��˽���ͨ�ŵ��׽���AcceptSocket
					if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
					{
						// �����׽���AcceptSocketΪ������ģʽ
						// �����������ڵ���WSASend()������������ʱ�Ͳ��ᱻ����
						NonBlock = 1;
						if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
						{
							printf("ioctlsocket()   failed   with   error   %d\n", WSAGetLastError());
							return -1;
						}
						// �����׽�����Ϣ����ʼ��LPSOCKET_INFORMATION�ṹ�����ݣ���AcceptSocket��ӵ�SocketArray������
						if (CreateSocketInformation(AcceptSocket) == FALSE)
							return -1;
					}
					else
					{
						if (WSAGetLastError() != WSAEWOULDBLOCK)
						{
							printf("accept()   failed   with   error   %d\n", WSAGetLastError());
							return -1;
						}
					}
				}
				else   // ��������
				{
					// �����ǰ�׽�����ReadSet�����У���������׽������п��Զ�ȡ������
					if (FD_ISSET(SocketInfo->Socket, &ReadSet))
					{
						Total--;				// ����һ�����ھ���״̬���׽���
						memset(SocketInfo->Buffer, ' ', DATA_BUFSIZE);			// ��ʼ��������
						SocketInfo->DataBuf.buf = SocketInfo->Buffer;			// ��ʼ��������λ��
						SocketInfo->DataBuf.len = DATA_BUFSIZE;				// ��ʼ������������
																			// ��������
						DWORD  Flags = 0;
						if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,NULL, NULL) == SOCKET_ERROR)
						{
							// ����������WSAEWOULDBLOCK��ʾ��û�����ݣ������ʾ�����쳣
							if (WSAGetLastError() != WSAEWOULDBLOCK)
							{
								printf("WSARecv()   failed   with   error   %d\n", WSAGetLastError());
								FreeSocketInformation(i);		// �ͷ��׽�����Ϣ
							}
							continue;
						}
						else   // ��������
						{
							SocketInfo->BytesRECV = RecvBytes;		// ��¼�������ݵ��ֽ���							
							if (RecvBytes == 0)									// ������յ�0���ֽڣ����ʾ�Է��ر�����
							{
								FreeSocketInformation(i);
								continue;
							}
							else															// ����ɹ��������ݣ����ӡ�յ�������
							{
								printf("%s\n",SocketInfo->DataBuf.buf);
							}
						}
					}
				}
			}
			else
			{
				// �����ǰ�׽�����WriteSet�����У���������׽��ֵ��ڲ����ݻ������������ݿ��Է���
				if (FD_ISSET(SocketInfo->Socket, &WriteSet))
				{
					Total--;			// ����һ�����ھ���״̬���׽���
					SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;			// ��ʼ��������λ��
					SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;	// ��ʼ������������
					if (SocketInfo->DataBuf.len > 0)		// �������Ҫ���͵����ݣ���������
					{
						if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,NULL, NULL) == SOCKET_ERROR)
						{
							// ����������WSAEWOULDBLOCK��ʾ��û�����ݣ������ʾ�����쳣
							if (WSAGetLastError() != WSAEWOULDBLOCK)
							{
								printf("WSASend()   failed   with   error   %d\n", WSAGetLastError());
								FreeSocketInformation(i);		// �ͷ��׽�����Ϣ
							}
							continue;
						}
						else
						{
							SocketInfo->BytesSEND += SendBytes;			// ��¼�������ݵ��ֽ���
																		// ����ӿͻ��˽��յ������ݶ��Ѿ����ص��ͻ��ˣ��򽫷��ͺͽ��յ��ֽ���������Ϊ0
							if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
							{
								SocketInfo->BytesSEND = 0;
								SocketInfo->BytesRECV = 0;
							}
						}
					}
				}
			}	// ���ListenSocketδ���������ҷ��صĴ�����WSAEWOULDBLOCK���ô����ʾû�н��յ��������󣩣�������쳣
		} 
	}
	// ��ͣ����������˳�
	system("pause");
	return 0;
}

