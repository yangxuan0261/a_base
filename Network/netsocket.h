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

#define BUFF_SIZE	1 << 20  // 1024*1024 = 1M
typedef unsigned int uint32;

struct SBuff
{
	SBuff() {
		//mBuff = new char[BUFF_SIZE] {0};
		mLen = BUFF_SIZE;
		mCurrPos = 0;
		mOpPos = 0;
		mOpLen = 0;
	}
	char mBuff[BUFF_SIZE];
	uint32 mLen;
	uint32 mCurrPos;
	uint32 mOpPos;
	uint32 mOpLen;

	void Recv(const char* _src, int _len);
	void Read(char* _dst, uint32* _len);
	bool HasOpData() { return mOpLen > 0; }
};

class CNetSocket
{
public:
	SOCKET	m_socket;		//平台相关的套接字

	//收数据
	int _Recv(char* _buff, int* _len);
	//发数据
	int _Send(const char *buf,int len);
public:
	CNetSocket();
	~CNetSocket();

	//初始化
	bool Initialize();

	//连接服务器
	bool Connect(const char *szAddr,int port,int& errorCode,unsigned long ip = 0);

	//bool GetLocalAddr (char *addr, short *port,unsigned long *ip = NULL);
	//bool GetRemoteAddr(char *addr,short *port,unsigned long *ip = NULL);

	//关闭套接字
	bool Close();
};

#endif