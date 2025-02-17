#include "StdAfx.h"
#include ".\wtsocket.h"

HANDLE g_UCSocketEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };	// read/write

static WTSocketCommand sWTSocket;
DWORD IPCSendCommandToUC(INT cmdId, LPCWSTR s1)
{
	LOG(L"IPCSendCommandToUC");
	sWTSocket.Send(s1);
	return 0;
}


DWORD IPCSetCallbackVS(OnCommandToVSProto fp)
{
//	g_WTGlobals.OpenLog(L"IPC_VS");
	LOG(L"IPCSetCallbackVS");
	OnCommandVS = fp;
	if ( OnCommandVS != NULL )
	{
		g_UCSocketEvent[0] = CreateEvent(NULL, TRUE, TRUE, "ReadUCSocketEvent");
		g_UCSocketEvent[1] = CreateEvent(NULL, TRUE, TRUE, "WriteUCSocketEvent");

		sWTSocket.ConnectClient();
		sWTSocket.WaitForState(S_CONNNECTED, 1000); // Allow client to connect
		Sleep(500); // Let the game finish initializing before we st any breakpoints
	}
	return 0;
}

OnCommandToVSProto OnCommandVS = NULL;

LPCWSTR WTSocketCommand::Read()
{
	ResetEvent(g_UCSocketEvent[0]);

	if(SANITY_KEY == ReadDWORD())
	{
		INT cmdID = ReadDWORD();
		DWORD dw1 = ReadDWORD();
		DWORD dw2 = ReadDWORD();
		WCHAR s1[1024], s2[1024];
		ReadStr(s1, 1024);
		ReadStr(s2, 1024);
		OnCommandVS(cmdID, dw1, dw2, s1, s2);
	}
	else
	{
		LOG(L"Error: WTSocketCommand::Read");
	}

	SetEvent(g_UCSocketEvent[0]);
	return 0;
}

void WTSocketCommand::Send( LPCWSTR cmdStr /*= NULL*/ )
{
	ResetEvent(g_UCSocketEvent[1]);

	SendDWORD(SANITY_KEY);
	SendStr(cmdStr);

	SetEvent(g_UCSocketEvent[1]);
}

void WTSocketCommand::ConnectClient()
{
	LOG(L"Connect");
	isServer = FALSE;
	InitSocket();
	StartReader();
	WaitForState(S_CONNECTING);
}

