#include "StdAfx.h"
#include ".\sockets.h"

extern HANDLE g_UCSocketEvent[2];
extern HANDLE g_VSSocketEvent[2];	// read/write

void socketError(WCHAR* str)
{
	LOG(L"socketError", str);
	WCHAR msg[512];
	SPRINTFW(msg, L"%s 0x%08x", str, WSAGetLastError());
	MessageBoxW(NULL, msg, L"SERVER SOCKET ERROR", MB_OK);
};
class RcvThread
{
	SocketSender *sock;
public:
	RcvThread(SocketSender *sock)
	{
		this->sock = sock;
	}
	~RcvThread()
	{
		sock->Close();
	}
};
DWORD CALLBACK Reciever(LPVOID lpThreadParameter)
{
	LOG(L"Reciever");
	SocketSender *sock = (SocketSender*)lpThreadParameter;
	RcvThread rcv(sock);
	sock->RecieverThread();
	return NULL;
}



CSocket::CSocket()
{
	s = NULL;
	state = S_NONE;
	LOG(L"CSocket");
	state = S_NONE;
}

void CSocket::SetState( int state )
{
	this->state = state;
}

void CSocket::ConnectServer()
{
	LOG(L"Connect");
	isServer = TRUE;
	InitSocket();
	StartReader();
	WaitForState(S_BOUND);
}

BOOL CSocket::WaitForState( int wstate, int timeout /*= 0*/ )
{
	for(int i = 0; state < wstate; i++)
	{
		LOG(L"WaitForState...");
		Sleep(100);
		if(timeout && timeout <= (i*100))
			return FALSE;
	}
	return TRUE;
}

int CSocket::InitSocket()
{
	LOG(L"InitSocket");
	WORD sockVersion = MAKEWORD(1,1);
	//start dll
	WSADATA wsaData;
	WSAStartup(sockVersion, &wsaData);

	//create socket
	s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if(s == INVALID_SOCKET)
	{
		socketError(L"Failed socket()");
		WSACleanup();
		return SERVER_SOCKET_ERROR;
	}
	return SOCKET_OK;
}

int CSocket::InitClient()
{
	LOG(L"InitClient");
	int rVal;
	SOCKADDR_IN serverInfo;

	serverInfo.sin_family = PF_INET;
	//store information about the server
	LOG(L"Getting Target", g_WTGlobals.WT_TARGET);
	char target[512];
	int i = 0;
	for(; g_WTGlobals.WT_TARGET[i]; i++)
		target[i] = (char)g_WTGlobals.WT_TARGET[i];
	target[i] = '\0';
	LPHOSTENT hostEntry = gethostbyname(target);
	if(!hostEntry)
	{
		LOG(L"gethostbyname failed.");
		return SOCKET_ERROR;
	}

	LOG(L"Got Target", g_WTGlobals.WT_TARGET);

	serverInfo.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);
	int port = _wtoi(g_WTGlobals.WT_PORT);
	serverInfo.sin_port = htons((USHORT)port);

	DOUBLE dSecondsWaiting=0;
	rVal=connect(s,(LPSOCKADDR)&serverInfo, sizeof(serverInfo));
	while(rVal && dSecondsWaiting < 10.f)
	{
		state = S_CONNECTING;
		LOG(L"Waiting for server...");
		Sleep(500);
		dSecondsWaiting += 0.5f;
		rVal=connect(s,(LPSOCKADDR)&serverInfo, sizeof(serverInfo));
	}

	client = s;
	if(rVal == S_OK)
	{
		state = S_CONNNECTED;
	}
	else
	{
		state = S_NONE;
	}
	return rVal;
}

void CSocket::StartReader()
{
	LOG(L"StartReader");
	DWORD threadID;
	CreateThread(NULL, 0, Reciever, (LPVOID)this, NULL, &threadID);
}

DWORD CALLBACK CSocket::RecieverThread()
{
	LOG(L"RecieverThread");
	try
	{
		int iResult=SOCKET_ERROR;
		if(isServer)
		{
			iResult = InitServer();
		}
		else
		{
			iResult = InitClient();
		}
		if ( iResult == SOCKET_OK )
		{
			state = S_CONNNECTED;
			while(state == S_CONNNECTED)
			{
				Read();
			}
		}
		else
		{
 			::SetEvent(isServer ? g_VSSocketEvent[0] : g_UCSocketEvent[0]);
			::SetEvent(isServer ? g_VSSocketEvent[1] : g_UCSocketEvent[1]);
		}
		/*
		if(isServer)
		{
			LOG(L"Client detached, waiting for new client...");
			WaitForClient(); // wait gor debugger to attach again
			LOG(L"Client reattached.");
			goto readagain;
		}
		*/
	}
	catch(...)
	{
	}
	state = S_CLOSED;
	return 1;
}

int CSocket::InitServer()
{
	LOG(L"InitServer");
	int rVal;
	SOCKADDR_IN sin;
	sin.sin_family = PF_INET;
	int port = _wtoi(g_WTGlobals.WT_PORT);
	swscanf(g_WTGlobals.WT_PORT, L"%d", &port);
	sin.sin_port = htons((USHORT)port);
	sin.sin_addr.s_addr = INADDR_ANY;
	char target[512];
	LOG(L"Getting Target", g_WTGlobals.WT_TARGET);
	int i = 0;
	for(; g_WTGlobals.WT_TARGET[i]; i++)
		target[i] = (char)g_WTGlobals.WT_TARGET[i];
	target[i] = '\0';

	LPHOSTENT hostEntry = gethostbyname(target);
	LOG(L"GotTarget", g_WTGlobals.WT_TARGET);
	if(!hostEntry)
	{
		LOG(L"gethostbyname failed.");
		return SOCKET_ERROR;
	}
	sin.sin_addr = *((LPIN_ADDR)*hostEntry->h_addr_list);

	//bind the socket
	rVal = bind(s, (LPSOCKADDR)&sin, sizeof(sin));
	if(rVal == SOCKET_ERROR)
	{
		socketError(L"Failed bind()");
		WSACleanup();
		return SERVER_SOCKET_ERROR;
	}

	//get socket to listen 
	rVal = listen(s, 2);
	if(rVal == SOCKET_ERROR)
	{
		socketError(L"Failed listen()");
		WSACleanup();
		return SERVER_SOCKET_ERROR;
	}
	state = S_BOUND;
	return WaitForClient();
}

int CSocket::WaitForClient()
{
	//wait for a client
	state = S_BOUND;
	LOG(L"waiting for newclient");

	client = accept(s, NULL, NULL);

	LOG(L"newclient found");

	if(client == INVALID_SOCKET)
	{
		socketError(L"Failed accept()");
		state = S_NONE;
		WSACleanup();
		return SERVER_SOCKET_ERROR;
	}
	state = S_CONNNECTED;
	return SOCKET_OK;
}

CSocket::~CSocket()
{
// 	Sleep(100);
	if ( state && state != S_CLOSED )
	{
		Close();
// 		Sleep(500);
		// 			WaitForState(S_CLOSED);
	}
}

BOOL CSocket::Close()
{
	if ( state && state != S_CLOSED && state != S_CLOSING )
	{
		state = S_CLOSING;
		closesocket(client);
		closesocket(s);

		WSACleanup();
		LOG(L"closing down");
	}
	WaitForState(S_CLOSED, 1000);
	return SOCKET_OK;
}

void SocketSender::SendStr( LPCWSTR str )
{
	if(state == S_CONNNECTED)
	{
		if(str && *str)
			send(client, (LPCSTR)str, (int)wcslen(str)*2, 0);
		const WCHAR cr = '\n';
		send(client, (LPCSTR)&cr, 2, 0);
		LOG(L"Sending ", str);
	}
}

void SocketSender::SendDWORD( DWORD val )
{
	if(state == S_CONNNECTED)
	{
		send(client, (LPCSTR)&val, 2, 0);
	}
}

DWORD SocketSender::ReadDWORD()
{
	DWORD dwVal = 0;
	if ( state != S_CLOSED && state != S_CLOSING )
	{
		int rVal = recv(client, (LPSTR)&dwVal, 2, 0);
		if(rVal == SOCKET_ERROR)
		{
			state = S_CLOSING;
			return 0;
		}
	}
	return dwVal;
}

LPCWSTR SocketSender::ReadStr( LPWSTR buf, int len )
{
	LOG(L"ReadStr");
	buf[0] = 0;
	if ( state != S_CLOSED && state != S_CLOSING )
	{
		int i = 0;
		int rVal;
		for(; i < len; i++)
		{
			rVal = recv(client, (LPSTR)&buf[i], 2, 0);
			if(rVal == SOCKET_ERROR || buf[i] == '\n')
				break;
		}
		buf[i] = 0;
		if(rVal != SOCKET_ERROR)
			LOG(L"Recieved ", buf);
		else
		{
			state = S_CLOSING;
			LOG(L"Error: SocketSender::ReadStr");
		}
	}
	return buf;
}

