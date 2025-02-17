//////////////////////////////////////////////////////////////////////////
// This file Loads the IPC/Socket dll to communicate with the IDE

#define GETPROCADDRESS GetProcAddress


#include "WTGlobals.h"

typedef DWORD (*OnCommandToUCProto)(INT, LPCWSTR);
typedef DWORD (*OnCommandToVSProto)(INT, DWORD, DWORD, LPCWSTR, LPCWSTR);
typedef void (*SetCallbackProtoUC)(OnCommandToUCProto);
typedef void (*SetCallbackProtoVS)(OnCommandToVSProto);

DWORD OnCommandToUC(INT cmdId, LPCWSTR cmdStr);
OnCommandToVSProto IPCSendCommandToVS = NULL;
class IPC_UC
{
	HMODULE hLib;
public:
#pragma warning(push)
#pragma warning(disable:4191) // disable: unsafe conversion from 'type of expression' to 'type required'
	IPC_UC()
	{
		hLib = LoadLibraryW(g_WTGlobals.WT_INTERFACEDLL);
		SetCallbackProtoUC SetCallback = (SetCallbackProtoUC) GETPROCADDRESS(hLib, "IPCSetCallbackUC");

		if ( SetCallback != NULL )
		{
			SetCallback(OnCommandToUC);
		}
		IPCSendCommandToVS = (OnCommandToVSProto) GETPROCADDRESS(hLib, "IPCSendCommandToVS");
	}
	~IPC_UC()
	{
		SendCommandToVS(CMD_GameEnded);
		Sleep(500);
		if ( hLib != NULL )
		{
			IPCSendCommandToVS = NULL;
			FreeLibrary(hLib);
		}
		hLib = NULL;
	}
#pragma warning(pop)

	DWORD SendCommandToVS( INT cmdId, DWORD dw1 = 0, DWORD dw2 = 0, LPCWSTR s1 = NULL, LPCWSTR s2 = NULL )
	{
		return IPCSendCommandToVS ? IPCSendCommandToVS(cmdId, dw1, dw2, s1, s2) : 0;
	}
	DWORD SendCommandToVS( INT cmdId, LPCWSTR s1, LPCWSTR s2 = NULL )
	{
		return IPCSendCommandToVS ? IPCSendCommandToVS(cmdId, 0, 0, s1, s2) : 0;
	}
	void Close()
	{
	}
};
