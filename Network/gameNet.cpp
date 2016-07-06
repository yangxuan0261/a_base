#include "gameNet.h"
#include <lua.h>
#include <lauxlib.h>
#include <iostream>

#define CACHE_SIZE 0x1000

CGameNet::CGameNet()
: mNetSocket(NULL)
//, mSendThRunnable(false)
//, mRecvThRunnable(false)
{

}

CGameNet::~CGameNet()
{

}

CGameNet* CGameNet::GetInstance()
{
	static CGameNet* instance = NULL;
	if (instance == NULL)
	{
		instance = new CGameNet();
	}
	return instance;
}

void CGameNet::_start()
{
	std::thread thSend(SenderThread, this);
	std::thread thRecv(ReceiverThread, this);
	thSend.detach();
	thRecv.detach();
}

void CGameNet::_sendProc()
{
	std::string buff("");
	while (mSendThRunnable)
	{
		mSendMtx.lock();
		if (!mSendList.empty())
		{
			buff = *mSendList.begin();
			mSendList.pop_front();
		}
		else
			buff = "";
		mSendMtx.unlock();

		if (buff == "")
		{
			Sleep(100);
		}
		else
		{
			bool ret = mNetSocket->Send(buff.c_str(), buff.length());
			if (!ret)
			{
				NotifyDisconnected();
				continue;
			}
		}
	}
}

void CGameNet::_recvProc()
{
	while (mRecvThRunnable)
	{
		bool ret;
		std::string buffer("");
		ret = mNetSocket->Recv(buffer, CACHE_SIZE);

		if (!ret)
		{
			NotifyDisconnected();
			continue;
		}
		else
		{
			if (buffer.length() > 0)
			{
				mRecvMtx.lock();
				mRecvList.push_back(buffer);
				mRecvMtx.unlock();
			}
			else
			{
				Sleep(100);
			}
		}
	}
}

bool CGameNet::_connect(std::string _ip, int _port)
{
	if (mNetSocket == NULL)
	{
		mNetSocket = new CNetSocket();
	}

	bool result = true;
	if (!mNetSocket->Initialize())
		result = false;

	int errCode = 0;
	if (result && mNetSocket->Connect(_ip.c_str(), _port, errCode))
	{
		printf("--------- connect to %s:%d success\n", _ip.c_str(), _port);
		mSendThRunnable = true;
		mRecvThRunnable = true;
		_start(); //链接成功开始工作线程

		if (mOnConnectDlg)
			mOnConnectDlg(true);


		return true;
	}
	else
	{
		printf("--------- connect fail, errCode:%f\n", errCode);
		//mSendThRunnable = false;
		//mRecvThRunnable = false;
		if (mOnConnectDlg)
			mOnConnectDlg(false);
		return false;
	}
}

void CGameNet::Connect(std::string _ip, int _port)
{
	SConnectEntity* entity = new SConnectEntity();
	entity->mNet = this;
	entity->mIp = _ip;
	entity->mPort = _port;

	std::thread thConnect(ConnectThread, entity);
	thConnect.detach();
	printf("---- thConnect:0x%x", &thConnect);
}

void CGameNet::Send(const char* _msg)
{
	mSendMtx.lock();
	mSendList.push_back(std::string(_msg));
	mSendMtx.unlock();
}

int CGameNet::Recv(std::string& _content)
{
	int ret = 0;
	mRecvMtx.lock();
	if (!mRecvList.empty())
	{
		ret = 1;
		_content = *mRecvList.begin();
	}
	mRecvMtx.unlock();
	return ret;
}

void CGameNet::Close()
{
	mSendThRunnable = false;
	mRecvThRunnable = false;
	mSendList.clear();
	mRecvList.clear();

	if (mNetSocket)
	{
		mNetSocket->Close();
		delete mNetSocket;
		mNetSocket = NULL;
	}
}

void CGameNet::NotifyDisconnected()
{
	printf("--- NotifyDisconnected\n");
	//mSendThRunnable = false;
	//mRecvThRunnable = false;

	if (mOnDisconnectDlg)
		mOnDisconnectDlg(false);
}

void CGameNet::ReceiverThread(void* _data)
{
	std::cout << "--- ReceiverThread start\n" << std::endl;
	CGameNet* instance = static_cast<CGameNet*>(_data);
	instance->_recvProc();
}

void CGameNet::SenderThread(void* _data)
{
	printf("--- SenderThread start\n");
	CGameNet* instance = static_cast<CGameNet*>(_data);
	instance->_sendProc();
}

void CGameNet::ConnectThread(void* _data)
{
	SConnectEntity* entity = static_cast<SConnectEntity*>(_data);
	CGameNet* gameNet = entity->mNet;
	gameNet->_connect(entity->mIp, entity->mPort);
	delete entity;
}

extern "C" {
	void Connect(const char* _ip, int _port)
	{
		CGameNet::GetInstance()->Connect(_ip, _port);
	}

	void Send(const char* _msg) {
		CGameNet::GetInstance()->Send(_msg);
	}

	void Close() {
		CGameNet::GetInstance()->Close();
	}
}



