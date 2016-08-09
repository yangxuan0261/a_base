#ifndef __GAME_NET_H__
#define __GAME_NET_H__

#include <thread>         // std::thread
#include <mutex> 
#include <list>
#include "netsocket.h"
#include <functional>

//typedef std::function<void(int)> Callback;
typedef void(*Callback)(int);
class CGameNet;

enum class ENetState : int
{
	None = 0,
	StartConnect,
	ConnectSucc,
	ConnectFail,
	Networking,
	Disconnected,
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
	bool Connect(std::string _ip, int _port);
	void Send(const char* _msg, int _len);
	void Recv(char* _buff, uint32* _len);
	void Close();
	void Update(float _dt); //main thread update state

	void _start();
	void _changeState(ENetState _st) { mNetState = _st; }
	void _reconnect();
	virtual void _sendProc();
	virtual void _recvProc();
	virtual bool _connect(std::string _ip, int _port);
	virtual void NotifyDisconnected();

	void SetCallback(Callback _cb) { mNetCallback = _cb; }

private:
	std::mutex	mSendMtx;
	std::mutex	mRecvMtx;
	SBuff*		mSendBuff;
	SBuff*		mRecvBuff;
	bool		mSendThRunnable;
	bool		mRecvThRunnable;
	Callback	mNetCallback;
	ENetState	mNetState;

	std::string mIp;
	int			mPort;
	std::string mConnectCb;
	std::string mDisconnectCb;

	CNetSocket* mNetSocket;
public:
	static void ReceiverThread(void* _data);
	static void SenderThread(void* _data);
	static bool ConnectThread(void* _data);
};

#endif