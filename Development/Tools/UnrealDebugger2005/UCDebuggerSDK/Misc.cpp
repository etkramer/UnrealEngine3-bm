#include "stdafx.h"
#include "Engine.h"
#include "Program.h"
// #include "Event.h"
#include "Breakpoint.h"
#include "StackFrame.h"
#include "Context.h"

WTGlobals g_WTGlobals(L"UCDebuggerSDK");


IPC_VS *g_pUCDebgger = NULL;
BOOL g_hadBreak = FALSE;
OnCommandToUCProto IPCSendCommandToUC = NULL;

IPC_VS::~IPC_VS()
{
	g_pUCDebgger = NULL;
	IPCSendCommandToUC = NULL;
	if ( hLib != NULL )
	{
		FreeLibrary(hLib);
		hLib = NULL;
	}
}

void PostDebugMessage(LPCWSTR msg)
{
	PostThreadMessage(g_mainThread, WM_OUTPUTDEBUGSTRING, (WPARAM)new CComBSTR(msg), 0);

}
BOOL isUCReady = FALSE;

void SendCommandToUC(INT cmdId, LPCWSTR cmd)
{
	if ( g_pUCDebgger != NULL )
	{
		int n = 20;
		for(;!isUCReady && n;n--)
		{
			Sleep(100);
		}
#ifdef _DEBUG
		ATLTRACE("SendCommandToUC");
		CComBSTR msg("SendCommandToUC ");
		msg += cmd;
		PostDebugMessage(msg);
#endif // _DEBUG
		g_pUCDebgger->SendCommandToUC(cmdId, cmd);
	}
}
DWORD OnCommandToVS(INT cmdId, DWORD dw1, DWORD dw2, LPCWSTR s1, LPCWSTR s2)
{
#ifdef _DEBUG
// 	WCHAR buf[2056];
// 	wsprintfW(buf, L"OnCommandToVS %d - %s\n", cmdId, s1);
// 	PostDebugMessage(buf);
#endif // _DEBUG
	ATLTRACE("OnCommandToVS");
	isUCReady = TRUE;

	if(cmdId == CMD_ShowDllForm)
	{
//		PostDebugMessage(L"CMD_ShowDllForm");
		// Breakpoint hit?
		g_hadBreak = TRUE;
		if ( g_CallStack->HasNodes() )
		{
			PostThreadMessage(g_mainThread, WM_BREAK, 0, 0);
		}
	}

	if(cmdId == CMD_CallStackClear)
	{
		// 		PostDebugMessage(L"CMD_CallStackClear");
		g_CallStack->Clear();
	}
	if(cmdId == CMD_AddLineToLog)
	{
		PostDebugMessage(s1);
	}
	if(cmdId == CMD_GameEnded)
	{
		LOG(L"CMD_GameEnded");
		PostThreadMessage(g_mainThread, WM_CLOSE, 0, 0);
	}
	if(cmdId == CMD_DebugWindowState)
	{
		PostDebugMessage(L"CMD_DebugWindowState");
		PostDebugMessage(s1);
	}
	if(cmdId == CMD_CallStackAdd)
	{
		// 		PostDebugMessage(L"CMD_CallStackAdd");
		g_CallStack->Add(s1, dw1);
	}
	wprintf(L"%d %s\n", cmdId, s1);
	return 1;
}

CComBSTR GetFileFromClass(LPCWSTR PkgClsMeth)
{
	WCHAR pkg[512];
	WCHAR cls[512];
	int i = 0;
	for(i=0;PkgClsMeth[i] && PkgClsMeth[i] != '.';i++)
	{
		if(PkgClsMeth[i] == ' ')
		{
			PkgClsMeth = &PkgClsMeth[i+1];
		}
	}
	for(i=0;PkgClsMeth[i] && PkgClsMeth[i] != '.';i++)
	{
		pkg[i] = PkgClsMeth[i];
	}
	pkg[i] = '\0';

	if(PkgClsMeth[0])
	{
		PkgClsMeth = &PkgClsMeth[i+1];
	}
	for(i=0;PkgClsMeth[i] && PkgClsMeth[i] != '.' && PkgClsMeth[i] != ':';i++)
	{
		cls[i] = PkgClsMeth[i];
	}
	cls[i] = '\0';

	CComBSTR file(g_WTGlobals.WT_GAMESRCPATH);
	file += pkg;
	file += L"\\Classes\\";
	file += cls;
	file += L".uc";
	LOG(L"GetFileFromClass", file);
	LOG(pkg, cls);

	return file;
}

void PkgClsFromFile(LPCWSTR file, CComBSTR &pkg, CComBSTR &cls)
{
	CComBSTR cwd, lwd;
	pkg = "";
	cls = "";
	INT i;
	for(i =0; file[i];i++)
	{
		if(file[i] == '\\' || file[i] == '.' || file[i] == ' ')
		{
			cwd.ToLower();
			if(cwd == L"classes")
			{
				pkg = lwd;
			}
			if(cwd == L"uc")
			{
				cls = lwd;
			}
			lwd = cwd;
			cwd.Empty();
		}
		else
		{
			cwd.Append(file[i]);
		}
	}
	cls = lwd;
	LOG(L"PkgClsFromFile", file);
	LOG(pkg, cls);

}
#include <fstream>
void LogToFile(LPCTSTR pPrompt)
{
	static std::ofstream logfile;
	if (!logfile.is_open())
	{
		//@todo - use the Binaries/WTDebugger directory
		logfile.open("c:\\UCDebuggerSDK.log",std::ios_base::app);
	}
	if (logfile.is_open())
	{
		logfile << pPrompt << std::endl;
		logfile.flush();
	}

}

