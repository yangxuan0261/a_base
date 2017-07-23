/*
netsocket.cpp: implementation of the CNetSocket class.
*/

#include "netsocket.h"

#include <stdio.h>

#define CACHE_SIZE 0x1000

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
	WSADATA wsaData;
	int nRet;
	if ((nRet = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0){
		printf("--- WSAStartup failed\n");
		return false;
	}
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_socket == SOCKET_ERROR)
	{
		int error = GETERROR;
		printf("--- init socket failed,for:%d ", error);
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
int CNetSocket::_Recv(char* _buff, int* _len)
{
	char buffer[CACHE_SIZE];
	int ret = recv(m_socket, buffer, CACHE_SIZE, 0);
	if (ret == 0) {
		// close
		//printf("recv remote close\r\n");
		return ret;
	}
	else if (ret < 0)
	{
		if (errno == EAGAIN || errno == EINTR) {
			return 0;
		}
		printf("socket error");
	}
	else
	{
		memcpy(_buff, buffer, ret);
		*_len = ret;
	}

	return ret;
}

/*
* return value
* =  0 send failed
* >  0	bytes send
* = -1 net dead
*/
int CNetSocket::_Send(const char *buf, int len)
{
	while (len > 0) {
		int r = send(m_socket, buf, len, 0);
		if (r < 0) {
			if (errno == EAGAIN || errno == EINTR)
				continue;
			printf("--- socket send error");
		}
		buf += r;
		len -= r;
	}

	return 1;
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

void SBuff::Recv(const char* _src, int _len)
{
	if (_len <= 0)
		return;

	uint32 rpos = mCurrPos + _len;
	if (rpos <= mLen)
	{
		memcpy(mBuff + mCurrPos, _src, _len);

		if (rpos == mLen)
			mCurrPos = 0;
		else
			mCurrPos += _len;
	}
	else //rest not enough, from head
	{
		uint32 num = mLen - mCurrPos;
		rpos = _len - num;

		memcpy(mBuff + mCurrPos, _src, num);
		mCurrPos = 0;
		memcpy(mBuff + mCurrPos, _src + num, rpos);
		mCurrPos += rpos;
	}

	mOpLen += _len;
}

void SBuff::Read(char* _dst, uint32* _len)
{
	if (mOpLen <= 0)
		return;

	uint32 rpos = mOpPos + mOpLen;
	if (rpos <= mLen)
	{
		memcpy(_dst, mBuff + mOpPos, mOpLen);
		memset(mBuff + mOpPos, 0x0, mOpLen);

		if (rpos == mLen)
			mOpPos = 0;
		else
			mOpPos += mOpLen;
	}
	else
	{
		uint32 num = mLen - mOpPos;
		rpos = mOpLen - num;

		memcpy(_dst, mBuff + mOpPos, num);
		memset(mBuff + mOpPos, 0x0, num);
		mOpPos = 0;
		memcpy(_dst + num, mBuff + mOpPos, rpos);
		memset(mBuff + mOpPos, 0x0, rpos);
		mOpPos += rpos;
	}

	*_len = mOpLen;
	mOpLen = 0;
}
