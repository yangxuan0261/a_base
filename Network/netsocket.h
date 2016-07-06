#ifndef __NET_SOCKET_H__
#define __NET_SOCKET_H__
#include <string>
#ifdef WIN32
/*
for windows
*/
#include <Winsock2.h>
#define GETERROR			WSAGetLastError()
#define CLOSESOCKET(s)		closesocket(s)
#define IOCTLSOCKET(s,c,a)  ioctlsocket(s,c,a)
#define CONN_INPRROGRESS	WSAEWOULDBLOCK
#define SHUT_RDWR			2
typedef int socklen_t;
#else
/*
for linux
*/
#include <sys/time.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
//typedef int            BOOL;
typedef unsigned char  BYTE;
typedef unsigned short WORD;
typedef unsigned int   DWORD;
#define TRUE  1
#define FALSE 0

/*
for socket
*/
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <sys/errno.h>
#include <arpa/inet.h>

typedef int SOCKET;
typedef sockaddr_in			SOCKADDR_IN;
typedef sockaddr			SOCKADDR;
#define INVALID_SOCKET	    (-1)
#define SOCKET_ERROR        (-1)
#define GETERROR			errno
#define WSAEWOULDBLOCK		EWOULDBLOCK
#define WSAEINTR			EINTR
#define CLOSESOCKET(s)		close(s)
#define IOCTLSOCKET(s,c,a)  ioctl(s,c,a)
#define CONN_INPRROGRESS	EINPROGRESS
#endif


class CNetSocket
{
private:
	SOCKET	m_socket;		//平台相关的套接字

	//收数据
	int _Recv(char *buf,int len);
	//发数据
	int _Send(const char *buf,int len);
public:
	CNetSocket();
	~CNetSocket();

	//初始化
	bool Initialize();

	// 绑定ip地址和端口
	bool BindAddr(char *ip,int port);

	//连接服务器
	bool Connect(const char *szAddr,int port,int& errorCode,unsigned long ip = 0);

	//收数据
	bool Recv(std::string& buf,int len);
	//发数据
	bool Send(const char *buf,int len);

	//bool GetLocalAddr (char *addr, short *port,unsigned long *ip = NULL);
	//bool GetRemoteAddr(char *addr,short *port,unsigned long *ip = NULL);

	//关闭套接字
	bool Close();
};

#endif