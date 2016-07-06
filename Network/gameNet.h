#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <thread>         // std::thread
#include <mutex> 
#include <list>
#include "netsocket.h"
#include <functional>

typedef std::function<void(bool)> Callback;

class CGameNet;

enum class ENetState : int
{
	None = 0,
	Connect,
	Disconnect,
	Reconnect,
	Count
};

struct SConnectEntity{
	CGameNet*	mNet;
	std::string mIp;
	int mPort;
};

class CGameNet
{
public:
	CGameNet();
	virtual ~CGameNet();

	static CGameNet* GetInstance();
	void Connect(std::string _ip, int _port);
	void Send(const char* _msg);
	int Recv(std::string& _content);

	void _start();
	virtual void Update() {}
	virtual void _sendProc();
	virtual void _recvProc();
	virtual bool _connect(std::string _ip, int _port);
	virtual void Close();
	virtual void NotifyDisconnected();

	void SetConnectDlg(Callback _cb) { mOnConnectDlg = _cb; }
	void SetDisconnectDlg(Callback _cb) { mOnDisconnectDlg = _cb; }

private:
	std::mutex mSendMtx;
	std::mutex mRecvMtx;
	std::list<std::string> mSendList;
	std::list<std::string> mRecvList;
	bool mSendThRunnable;
	bool mRecvThRunnable;
	Callback	mOnDisconnectDlg;
	Callback	mOnConnectDlg;

	CNetSocket* mNetSocket;
public:
	static void ReceiverThread(void* _data);
	static void SenderThread(void* _data);
	static void ConnectThread(void* _data);
};

#endif