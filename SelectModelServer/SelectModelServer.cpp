// TcpServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <WINSOCK2.H>   
#include <iostream>

#pragma comment(lib,"WS2_32.lib")   

#define   PORT   9990   
#define   DATA_BUFSIZE   8192   

// 定义套接字信息
typedef   struct   _SOCKET_INFORMATION {
	CHAR   Buffer[DATA_BUFSIZE];		// 发送和接收数据的缓冲区
	WSABUF   DataBuf;						// 定义发送和接收数据缓冲区的结构体，包括缓冲区的长度和内容
	SOCKET   Socket;							// 与客户端进行通信的套接字
	DWORD   BytesSEND;					// 保存套接字发送的字节数
	DWORD   BytesRECV;					// 保存套接字接收的字节数
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

DWORD   TotalSockets = 0;				// 记录正在使用的套接字总数量
LPSOCKET_INFORMATION   SocketArray[FD_SETSIZE];			// 保存Socket信息对象的数组，FD_SETSIZE表示SELECT模型中允许的最大套接字数量

														// 创建SOCKET信息
BOOL   CreateSocketInformation(SOCKET   s)
{
	LPSOCKET_INFORMATION   SI;										// 用于保存套接字的信息       
																	//   printf("Accepted   socket   number   %d\n",   s);			// 打开已接受的套接字编号
																	// 为SI分配内存空间
	if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR,
		sizeof(SOCKET_INFORMATION))) == NULL)
	{
		printf("GlobalAlloc()   failed   with   error   %d\n", GetLastError());
		return   FALSE;
	}
	// 初始化SI的值    
	SI->Socket = s;
	SI->BytesSEND = 0;
	SI->BytesRECV = 0;
	// 在SocketArray数组中增加一个新元素，用于保存SI对象 
	SocketArray[TotalSockets] = SI;
	TotalSockets++;						// 增加套接字数量

	return(TRUE);
}

// 从数组SocketArray中删除指定的LPSOCKET_INFORMATION对象
void   FreeSocketInformation(DWORD   Index)
{
	LPSOCKET_INFORMATION SI = SocketArray[Index];	// 获取指定索引对应的LPSOCKET_INFORMATION对象
	DWORD   i;
	// 关闭套接字
	closesocket(SI->Socket);
	//printf("Closing   socket   number   %d\n",   SI->Socket);   
	// 释放指定LPSOCKET_INFORMATION对象资源
	GlobalFree(SI);
	// 将数组中index索引后面的元素前移
	for (i = Index; i < TotalSockets; i++)
	{
		SocketArray[i] = SocketArray[i + 1];
	}
	TotalSockets--;		// 套接字总数减1
}

// 主函数，启动服务器
int _tmain(int argc, _TCHAR* argv[])
{
	SOCKET   ListenSocket;					// 监听套接字
	SOCKET   AcceptSocket;					// 与客户端进行通信的套接字
	SOCKADDR_IN   InternetAddr;			// 服务器的地址
	WSADATA   wsaData;						// 用于初始化套接字环境
	INT   Ret;											// WinSock API的返回值
	FD_SET   WriteSet;							// 获取可写性的套接字集合
	FD_SET   ReadSet;							// 获取可读性的套接字集合
	DWORD   Total = 0;								// 处于就绪状态的套接字数量
	DWORD   SendBytes;						// 发送的字节数
	DWORD   RecvBytes;						// 接收的字节数


											// 初始化WinSock环境
	if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
	{
		printf("WSAStartup()   failed   with   error   %d\n", Ret);
		WSACleanup();
		return -1;
	}
	// 创建用于监听的套接字 
	if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0,
		WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
		printf("WSASocket()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// 设置监听地址和端口号
	InternetAddr.sin_family = AF_INET;
	InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	InternetAddr.sin_port = htons(PORT);
	// 绑定监听套接字到本地地址和端口
	if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
	{
		printf("bind()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// 开始监听
	if (listen(ListenSocket, 5))
	{
		printf("listen()   failed   with   error   %d\n", WSAGetLastError());
		return -1;
	}
	// 设置为非阻塞模式
	ULONG NonBlock = 1;
	if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
	{
		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
		return -1;
	}
	// 为ListenSocket套接字创建对应的SOCKET_INFORMATION
	// 这样就可以把ListenSocket添加到SocketArray数组中
	CreateSocketInformation(ListenSocket);
	while (TRUE)
	{
		// 准备用于网络I/O通知的读/写套接字集合
		FD_ZERO(&ReadSet);
		FD_ZERO(&WriteSet);
		// 向ReadSet集合中添加监听套接字ListenSocket
		FD_SET(ListenSocket, &ReadSet);
		// 将SocketArray数组中的所有套接字添加到WriteSet和ReadSet集合中
		// SocketArray数组中保存着监听套接字和所有与客户端进行通信的套接字
		// 这样就可以使用select()判断哪个套接字有接入数据或者读取/写入数据
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];
			FD_SET(SocketInfo->Socket, &WriteSet);
			FD_SET(SocketInfo->Socket, &ReadSet);
		}
		// 判断读/写套接字集合中就绪的套接字    
		if ((Total = select(0, &ReadSet, &WriteSet, NULL, NULL)) == SOCKET_ERROR)
		{
			printf("select()   returned   with   error   %d\n", WSAGetLastError());
			return -1;
		}
		// 依次处理所有套接字。本服务器是一个回应服务器，即将从客户端收到的字符串再发回到客户端。
		for (DWORD i = 0; i < TotalSockets; i++)
		{
			LPSOCKET_INFORMATION SocketInfo = SocketArray[i];			// SocketInfo为当前要处理的套接字信息
																		// 判断当前套接字的可读性，即是否有接入的连接请求或者可以接收数据
			if (FD_ISSET(SocketInfo->Socket, &ReadSet))
			{
				if (SocketInfo->Socket == ListenSocket)		// 对于监听套接字来说，可读表示有新的连接请求
				{
					Total--;	// 就绪的套接字减1
								// 接受连接请求，得到与客户端进行通信的套接字AcceptSocket
					if ((AcceptSocket = accept(ListenSocket, NULL, NULL)) != INVALID_SOCKET)
					{
						// 设置套接字AcceptSocket为非阻塞模式
						// 这样服务器在调用WSASend()函数发送数据时就不会被阻塞
						NonBlock = 1;
						if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
						{
							printf("ioctlsocket()   failed   with   error   %d\n", WSAGetLastError());
							return -1;
						}
						// 创建套接字信息，初始化LPSOCKET_INFORMATION结构体数据，将AcceptSocket添加到SocketArray数组中
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
				else   // 接收数据
				{
					// 如果当前套接字在ReadSet集合中，则表明该套接字上有可以读取的数据
					if (FD_ISSET(SocketInfo->Socket, &ReadSet))
					{
						Total--;				// 减少一个处于就绪状态的套接字
						memset(SocketInfo->Buffer, ' ', DATA_BUFSIZE);			// 初始化缓冲区
						SocketInfo->DataBuf.buf = SocketInfo->Buffer;			// 初始化缓冲区位置
						SocketInfo->DataBuf.len = DATA_BUFSIZE;				// 初始化缓冲区长度
																			// 接收数据
						DWORD  Flags = 0;
						if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags,NULL, NULL) == SOCKET_ERROR)
						{
							// 错误编码等于WSAEWOULDBLOCK表示暂没有数据，否则表示出现异常
							if (WSAGetLastError() != WSAEWOULDBLOCK)
							{
								printf("WSARecv()   failed   with   error   %d\n", WSAGetLastError());
								FreeSocketInformation(i);		// 释放套接字信息
							}
							continue;
						}
						else   // 接收数据
						{
							SocketInfo->BytesRECV = RecvBytes;		// 记录接收数据的字节数							
							if (RecvBytes == 0)									// 如果接收到0个字节，则表示对方关闭连接
							{
								FreeSocketInformation(i);
								continue;
							}
							else															// 如果成功接收数据，则打印收到的数据
							{
								printf("%s\n",SocketInfo->DataBuf.buf);
							}
						}
					}
				}
			}
			else
			{
				// 如果当前套接字在WriteSet集合中，则表明该套接字的内部数据缓冲区中有数据可以发送
				if (FD_ISSET(SocketInfo->Socket, &WriteSet))
				{
					Total--;			// 减少一个处于就绪状态的套接字
					SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;			// 初始化缓冲区位置
					SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;	// 初始化缓冲区长度
					if (SocketInfo->DataBuf.len > 0)		// 如果有需要发送的数据，则发送数据
					{
						if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0,NULL, NULL) == SOCKET_ERROR)
						{
							// 错误编码等于WSAEWOULDBLOCK表示暂没有数据，否则表示出现异常
							if (WSAGetLastError() != WSAEWOULDBLOCK)
							{
								printf("WSASend()   failed   with   error   %d\n", WSAGetLastError());
								FreeSocketInformation(i);		// 释放套接字信息
							}
							continue;
						}
						else
						{
							SocketInfo->BytesSEND += SendBytes;			// 记录发送数据的字节数
																		// 如果从客户端接收到的数据都已经发回到客户端，则将发送和接收的字节数量设置为0
							if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
							{
								SocketInfo->BytesSEND = 0;
								SocketInfo->BytesRECV = 0;
							}
						}
					}
				}
			}	// 如果ListenSocket未就绪，并且返回的错误不是WSAEWOULDBLOCK（该错误表示没有接收的连接请求），则出现异常
		} 
	}
	// 暂停，按任意键退出
	system("pause");
	return 0;
}

