#pragma once

#include "stdafx.h"
#include <iostream>
#include <winsock.h>
#include <windows.h>
#include <tchar.h>
#include "DebuggerInterface.h"
#include "IPC.h"
#include "WTGlobals.h"

using namespace std;

#define SERVER_SOCKET_ERROR 1
#define SOCKET_OK 0

#pragma comment(lib, "wsock32.lib")

void socketError(WCHAR*);
DWORD CALLBACK Reciever(LPVOID lpThreadParameter);

enum {S_NONE, S_BOUND, S_CONNECTING, S_CONNNECTED, S_CLOSING, S_CLOSED};
class CSocket
{
protected:
	int state;
	SOCKET s;
	SOCKET client;
	BOOL isServer;
	BOOL isLoaded;
public:
	CSocket();
	void SetState(int state);
	void ConnectServer();
	BOOL WaitForState(int wstate, int timeout = 0);
	int InitSocket();
	int InitClient();
	int GetState(){ return state;}

	void StartReader();
	virtual LPCWSTR Read(){ return NULL; };
	DWORD CALLBACK RecieverThread();
	virtual void ConnectClient(){}
	int InitServer();
	int WaitForClient();
	~CSocket();
	BOOL Close();
};
class SocketSender : public CSocket
{
	WCHAR cmdBuf[1024];
public:
	void SendStr(LPCWSTR str);
	void SendDWORD(DWORD val);
	virtual DWORD ReadDWORD();
	virtual LPCWSTR ReadStr(LPWSTR buf, int len);
};


#define SANITY_KEY 0xdead
