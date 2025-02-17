#pragma once
#include "WTGlobals.h"


typedef DWORD (*OnCommandToUCProto)(INT, LPCWSTR);
typedef DWORD (*OnCommandToVSProto)(INT, DWORD, DWORD, LPCWSTR, LPCWSTR);
typedef void (*SetCallbackProtoUC)(OnCommandToUCProto);
typedef DWORD (*SetCallbackProtoVS)(OnCommandToVSProto);


DWORD OnCommandToVS(INT cmdId, DWORD dw1, DWORD dw2, LPCWSTR s1, LPCWSTR s2);
extern OnCommandToUCProto IPCSendCommandToUC;

class IPC_VS
{
	DWORD procID;
	HMODULE hLib;

public:
	IPC_VS()
	: procID(0), hLib(NULL)
	{
		hLib = LoadLibraryW(g_WTGlobals.WT_INTERFACEDLL);
		if ( hLib != NULL )
		{
			SetCallbackProtoVS SetCallback = (SetCallbackProtoVS) GetProcAddress(hLib, "IPCSetCallbackVS");
			if ( SetCallback != NULL )
			{
				procID = SetCallback(OnCommandToVS);
			}
			IPCSendCommandToUC  = (OnCommandToUCProto) GetProcAddress(hLib, "IPCSendCommandToUC");
		}
	}
	~IPC_VS();

	DWORD SendCommandToUC( INT cmdId, LPCWSTR cmd = NULL)
	{
		return IPCSendCommandToUC ? IPCSendCommandToUC(cmdId, cmd) : 0;
	}
	DWORD GetGameProcID()
	{
		return procID;
	}
};


