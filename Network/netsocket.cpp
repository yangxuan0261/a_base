/*
netsocket.cpp: implementation of the CNetSocket class.
*/

#include "netsocket.h"

#include <stdio.h>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

CNetSocket::CNetSocket() :m_socket(INVALID_SOCKET)
{
}

CNetSocket::~CNetSocket()
{
	Close();
}

bool CNetSocket::Initialize()
{

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == SOCKET_ERROR)
	{
		int error = GETERROR;
		printf("--- init socket failed,for:%d ", error);
		return false;
	}
	return true;
}

bool CNetSocket::BindAddr(char *ip, int port)
{
	SOCKADDR_IN addrLocal;
	addrLocal.sin_family = AF_INET;
	addrLocal.sin_port = htons(port);
	if (ip)
	{
		addrLocal.sin_addr.s_addr = inet_addr(ip);
	}
	else
	{
		addrLocal.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	if (bind(m_socket, (SOCKADDR *)&addrLocal, sizeof(addrLocal)) == SOCKET_ERROR)
	{
		int error = GETERROR;
		printf("socket bind:%s:%d failed,for:%d", ip, port, error);
		return false;
	}
	return true;
}

bool CNetSocket::Connect(const char *szAddr, int port, int& errorCode, unsigned long ip)
{
	SOCKADDR_IN addrRemote;
	hostent *host = NULL;
	errorCode = 0;

#if 0
	host = gethostbyname(szAddr);
	if(host == 0)
	{
		return false;
	}

	addrRemote.sin_family = host->h_addrtype;
	addrRemote.sin_port = ntohs((xc::u16)port);
	memcpy(&addrRemote.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

#else
	memset(&addrRemote, 0, sizeof(addrRemote));
	addrRemote.sin_family = AF_INET;
	addrRemote.sin_port = ntohs(port);

	if (szAddr)
		addrRemote.sin_addr.s_addr = inet_addr(szAddr);
	else
		addrRemote.sin_addr.s_addr = ip;

	if (addrRemote.sin_addr.s_addr == INADDR_NONE)
	{
		if (!szAddr) return false;
		host = gethostbyname(szAddr);
		if (!host) return false;
		memcpy(&addrRemote.sin_addr, host->h_addr_list[0], host->h_length);
	}
#endif
	if (connect(m_socket, (SOCKADDR *)&addrRemote, sizeof(addrRemote)) == SOCKET_ERROR)
	{
		errorCode = GETERROR;
		if (errorCode != CONN_INPRROGRESS)
		{
			printf("--- socket connect:%s:%d failed,for:%d\n", szAddr, port, errorCode);
			return false;
		}
	}
	return true;
}

/*
* return value
* =  0 recv failed
* >  0	bytes recv
* = -1 net dead
*/
int CNetSocket::_Recv(char *buf, int len)
{
	if (len < 0 || buf == NULL)
		return -1;

	int ret, err;
	/* in linux be careful of SIGPIPE */
	do {
		ret = recv(m_socket, buf, len, 0);
		if (ret == SOCKET_ERROR) {
			err = GETERROR;
		}
	} while (ret == SOCKET_ERROR && err == WSAEINTR);

	if (ret == 0)
	{
		/* remote closed */
		printf("recv remote close\r\n");
		return -1;
	}

	if (ret == SOCKET_ERROR)
	{
		int err = GETERROR;
		if (err != WSAEWOULDBLOCK)
		{
			printf("recv local close\r\n");
			return -1;
		}
	}
	return ret;
}

bool CNetSocket::Recv(std::string& buf,int len)
{
	/*
	int pos = 0;
	int remain = len;
	int ret;

	while (remain > 0)
	{
		if (pos >= len)
			return false;

		ret = _Recv(buf + pos, remain);

		if (ret < 0)
			return false;

		remain -= ret;
		if (remain < 0)
			return false;

		pos += ret;
	}
	*/
	char tmpBuf[65533];
	int ret = recv(m_socket, tmpBuf, len, 0);
	if (ret == 0)
	{
		return false;
	}
	else if (ret < 0)
	{
		return false;
	}
	else
	{
		buf = std::string(tmpBuf);
		return true;
	}

}

/*
* return value
* =  0 send failed
* >  0	bytes send
* = -1 net dead
*/
int CNetSocket::_Send(const char *buf, int len)
{
	if (len < 0 || buf == NULL)
		return -1;

	int ret, err;
	do {
		ret = send(m_socket, buf, len, 0);
		if (ret == SOCKET_ERROR) {
			err = GETERROR;
		}
	} while (ret == SOCKET_ERROR && err == WSAEINTR);

	if (ret == SOCKET_ERROR)
	{
		int err = GETERROR;
		if (err == WSAEWOULDBLOCK) return 0;
		//if (err==WSAECONNABORTED)
		//	CCLOG("send local close\r\n");
		//else
		//	CCLOG("send remote close\r\n");
		return -1;
	}
	return ret;
}

bool CNetSocket::Send(const char *buf, int len)
{
	int pos = 0;
	int remain = len;
	int ret;

	while (remain > 0)
	{
		if (pos >= len)
			return false;

		ret = _Send(buf + pos, remain);

		if (ret < 0)
			return false;

		remain -= ret;
		if (remain < 0)
			return false;

		pos += ret;
	}

	return true;
}

bool CNetSocket::Close()
{
	if (m_socket == INVALID_SOCKET)
		return false;
	shutdown(m_socket, SHUT_RDWR);
	CLOSESOCKET(m_socket);
	m_socket = INVALID_SOCKET;
	printf("--- socket close completed!\r\n");
	return true;
}