#include "StdAfx.h"
#include ".\sockets.h"
#include ".\ucsocket.h"
#include ".\WTGlobals.h"

HANDLE g_VSSocketEvent[2] = { INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE };	// read/write

static UCSocketCommand sUCSocket;
DWORD IPCSendCommandToVS( INT cmdId, DWORD dw1 /*= 0*/, DWORD dw2 /*= 0*/, LPCWSTR s1 /*= NULL*/, LPCWSTR s2 /*= NULL*/ )
{
	LOG(L"IPCSendCommandToVS");

	static FILE* fp = NULL;
	static FILE* fpLog = NULL;
	static int ID = 0;

	const int SocketState = sUCSocket.GetState();
	if(CMD_LockList == cmdId)
	{
		if(!fp)
		{
			LOG(L"Opening ", g_WTGlobals.WT_WATCHFILE);
			fp = _wfopen(g_WTGlobals.WT_WATCHFILE, L"wb");
		}
		ID = 0;
	}
	if(CMD_AddWatch == cmdId)
	{
		int id = ++ID; // dw2+1;
		if(fp)
		{
			dw2++;
			// 			fwrite(&dw2, sizeof(DWORD), 1, fp);
			if(s2 && s2[0])
				fwprintf(fp, L"%d %c %s {%s}\r\n", dw1, dw2, s1, s2);
			else
				fwprintf(fp, L"%d %c %s\r\n", dw1, dw2, s1);
		}
		return (id);
	}
	if(CMD_UnlockList == cmdId || CMD_ShowDllForm == cmdId)
	{
		if(fp)
			fclose(fp);
		fp = NULL;
	}
	
	if ( SocketState != S_CLOSING && SocketState != S_CLOSED )
	{
		while(sUCSocket.GetState() < S_CONNNECTED)
		{
			LOG(L"Wait for S_CONNNECTED...");
			Sleep(100);
		}

		sUCSocket.Send(cmdId, dw1, dw2, s1, s2);
	}
	return NULL;
}
void IPCSetCallbackUC(OnCommandToUCProto fp)
{
	OnCommandUC = fp;
	if ( OnCommandUC != NULL )
	{
//		g_WTGlobals.OpenLog(L"IPC_UC");
		LOG(L"IPCSetCallbackUC");

		g_VSSocketEvent[0] = CreateEvent(NULL, TRUE, TRUE, "ReadVSSocketEvent");
		g_VSSocketEvent[1] = CreateEvent(NULL, TRUE, TRUE, "WriteVSSocketEvent");

		sUCSocket.ConnectServer();
		sUCSocket.StartVSDebugger();
		sUCSocket.WaitForState(S_CONNNECTED, 1000); // Allow client to connect
	}
	else if ( sUCSocket.GetState() != S_CLOSED )
	{
		sUCSocket.Close();
	}
}
OnCommandToUCProto OnCommandUC = NULL;

UCSocketCommand::UCSocketCommand()
{
}
void UCSocketCommand::Send( INT cmdId, DWORD dw1 /*= 0*/, DWORD dw2 /*= 0*/, LPCWSTR s1 /*= NULL*/, LPCWSTR s2 /*= NULL */ )
{
	ResetEvent(g_VSSocketEvent[1]);

	SendDWORD(SANITY_KEY);
	SendDWORD(cmdId);
	SendDWORD(dw1);
	SendDWORD(dw2);
	SendStr(s1);
	SendStr(s2);

	SetEvent(g_VSSocketEvent[1]);
}

LPCWSTR UCSocketCommand::Read()
{
	ResetEvent(g_VSSocketEvent[0]);

	if(SANITY_KEY == ReadDWORD())
	{
		WCHAR s1[1024];
		ReadStr(s1, 1024);
#ifdef _DEBUG
		Send(CMD_AddLineToLog, 0, 0, s1);
#endif // _DEBUG

		if ( OnCommandUC != NULL )
		{
			OnCommandUC(0, s1);
		}

		// if the user wishes to continue execution, bring the game window to the foreground
		if(lstrcmpW(L"go", s1) == 0)
		{
			void BringAppToFront(DWORD appPID);
			BringAppToFront(::GetCurrentProcessId());
		}
	}
	else
	{
		WCHAR s1[1024];
		ReadStr(s1, 1024);
		LOG(L"Error: UCSocketCommand::Read", s1);
	}

	SetEvent(g_VSSocketEvent[0]);
	return 0;
}

UCSocketCommand::~UCSocketCommand()
{
	for ( INT i = 0; i < 2; i++ )
	{
		if ( g_VSSocketEvent[i] != NULL )
		{
			::CloseHandle(g_VSSocketEvent[i]);
		}
	}

	Sleep(500);
	Close();
}

void UCSocketCommand::StartVSDebugger()
{
#if 0
	// touch file to signal VS to attach
	WCHAR attachFile[1024];
	WTStr::WStrCpy(attachFile, g_WTGlobals.WT_TMPDIR);
	WTStr::WStrCat(attachFile, L"\\attach.txt");
	FILE* fp = _wfopen(attachFile, L"wb");
	fwprintf(fp, L"1\r\n");
	fclose(fp);

	if(!WaitForState(S_CONNNECTED, 1000))
	{
#ifndef _DEBUG
		// No connection for less than x seconds
		WCHAR projectFile[1024];
		WTStr::WStrCpy(projectFile, g_WTGlobals.WT_DLLPATH);
		WTStr::WStrCat(projectFile, L"..\\..\\Development\\Src\\UnrealEngine3.sln");
		ShellExecuteW(::GetDesktopWindow(), L"open", projectFile, NULL, NULL, SW_SHOWNORMAL);
#endif
	}
#endif
}
