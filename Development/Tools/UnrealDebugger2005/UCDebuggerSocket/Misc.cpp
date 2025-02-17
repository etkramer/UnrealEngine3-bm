#include "StdAfx.h"
#include ".\misc.h"
#include "wtglobals.h"

void BringWndToFront(HWND h)
{
	DWORD dwThreadId = GetCurrentThreadId();

	DWORD dwActiveThreadId = NULL;
	dwActiveThreadId = GetWindowThreadProcessId( h, NULL );

	AttachThreadInput( dwThreadId, dwActiveThreadId, TRUE );
	::SetForegroundWindow(h);
	AttachThreadInput( dwThreadId, dwActiveThreadId, FALSE );
}
static HWND hGame = NULL;
BOOL CALLBACK WindowEnumCB(HWND h, LPARAM appPID)
{
	DWORD pid = 0;
	GetWindowThreadProcessId(h, &pid);
	if(pid == appPID && ::IsWindowVisible(h))
	{
		char title[512];
		::GetWindowText(h, title, 511);
		if(strstr(title, "UnrealEngine"))
			hGame = h; // save this so we can bring it up last
		else
			BringWndToFront(h);
	}
	return TRUE;
}
void BringAppToFront(DWORD appPID)
{
	hGame = NULL; // reget game window
	EnumWindows(WindowEnumCB, appPID);
	if(hGame)
		BringWndToFront(hGame); // make game window the active window
}

DWORD LaunchProcess(LPCWSTR cmd)
{
	STARTUPINFOW sinfo;
	PROCESS_INFORMATION pinfo;
	ZeroMemory(&sinfo, sizeof(STARTUPINFOW));
	ZeroMemory(&pinfo, sizeof(PROCESS_INFORMATION));
	sinfo.cb = sizeof(STARTUPINFOW);
	sinfo.dwFlags = STARTF_USESHOWWINDOW;
	sinfo.wShowWindow = SW_SHOWNORMAL;
	WCHAR tcmd[1024];
	WTStr::WStrCpy(tcmd, cmd);
	if (CreateProcessW(NULL, tcmd, NULL, NULL, FALSE, 
		CREATE_NEW_PROCESS_GROUP|NORMAL_PRIORITY_CLASS, NULL, NULL, &sinfo, &pinfo))
	{
		return pinfo.dwProcessId;
	}
	MessageBoxW(NULL, cmd, L"Error Launching Process", MB_OK);
	return 0;
}
DWORD LaunchGame()
{
	WCHAR cmd[1024];
	wsprintfW(cmd, L"%s -autodebug -vadebug", g_WTGlobals.WT_GAMEPATH);
	return LaunchProcess(cmd);
}
