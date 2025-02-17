// FXenonConsole.cpp : Implementation of CFXenonConsole

#include "stdafx.h"
#include "FXenonConsole.h"
#include <assert.h>
#include <diacreate.h>

// global critical section for synchronizing calls into dbghelp.dll
CRITICAL_SECTION SymbolCS;

const wchar_t* EVENTTYPE_STR[] = 
{
	L"NoEvent",
	L"ExecutionBreak",
	L"DebugString",
	L"ExecStateChange",
	L"SingleStep",
	L"ModuleLoad",
	L"ModuleUnload",
	L"ThreadCreate",
	L"ThreadDestroy"
	L"Exception",
	L"AssertionFailed",
	L"AssertionFailedEx",
	L"DataBreak",
	L"RIP = DataBreak",
	L"SectionLoad",
	L"SectionUnload",
	L"StackTrace",
	L"FiberCreate",
	L"FiberDestroy",
	L"BugCheck",
};

const wchar_t* EXECSTATE[] =
{
	L"Stopped",
	L"Running",
	L"Rebooting",
	L"Pending",
	L"RebootingTitle",
	L"PendingTitle",
};

// Thread name code from MSDN that allows us to set the name of a thread and have it show up in the debugger

//
// Usage: SetThreadName (-1, "MainThread");
//
#define MS_VC_EXCEPTION 0x406D1388

typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;

void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = szThreadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD*)&info);
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}

// CFXenonConsole

// static symbol stuff
HMODULE CFXenonConsole::DbgLibraryHandle = 0;
HMODULE CFXenonConsole::XbdmLibraryHandle = 0;
HMODULE CFXenonConsole::DiaLibraryHandle = 0;

/**
 * Loads the dll's and function pointers to resolve symbols from call stacks.
 */
void CFXenonConsole::LoadSymbolHelpers()
{
	if(!DbgLibraryHandle || !XbdmLibraryHandle || !DiaLibraryHandle)
	{
		// Get the XEDK environment variable.
		wchar_t* XedkDir;
		size_t XedkDirSize;
		errno_t Err = _wdupenv_s(&XedkDir, &XedkDirSize, L"xedk");
		if(Err || !XedkDir)
		{
			OutputDebugStringW(L"Couldn't read xedk environment variable.\n");
			return;
		}

		// Create a fully specified path to the XEDK version of dbghelp.dll
		// This is necessary because symsrv.dll must be in the same directory
		// as dbghelp.dll, and only a fully specified path can guarantee which
		// version of dbghelp.dll we load.
		std::wstring XedkDirStr = XedkDir;
		std::wstring DbgHelpPath = XedkDirStr + L"\\bin\\win32\\dbghelp.dll";
		std::wstring XbdmPath = XedkDirStr + L"\\bin\\win32\\xbdm.dll";
		std::wstring DiaPath = XedkDirStr + L"\\bin\\win32\\msdia80.dll";

		// Free xedkDir
		if(XedkDir)
		{
			free(XedkDir);
			XedkDir = NULL;
		}

		// Call LoadLibrary on DbgHelp.DLL with our fully specified path.
		DbgLibraryHandle = LoadLibrary(DbgHelpPath.c_str());
		XbdmLibraryHandle = LoadLibrary(XbdmPath.c_str());
		DiaLibraryHandle = LoadLibrary(DiaPath.c_str());

		if(!XbdmLibraryHandle)
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolHelpers() ERROR: Couldn't load xbdm.dll from %xedk%\\bin\\win32.\n");
		}

		if(!DiaLibraryHandle)
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolHelpers() ERROR: Couldn't load msdia80.dll from %xedk%\\bin\\win32.\n");
		}

		// Print an error message and return FALSE if DbgHelp.DLL didn't load.
		if(!DbgLibraryHandle)
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolHelpers() ERROR: Couldn't load DbgHelp.dll from %xedk%\\bin\\win32.\n");
			return;
		}

		SymSetOptions(SYMOPT_UNDNAME | SYMOPT_LOAD_LINES | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_DEFERRED_LOADS | SYMOPT_DEBUG | SYMOPT_ALLOW_ABSOLUTE_SYMBOLS | SYMOPT_EXACT_SYMBOLS);
	}
}

// IXboxEvents functions

/**
 * Event handler for xbox events.
 *
 * @param	eEventCode		The ID of the event.
 * @param	pEventInfo		Extra information about the event.
 */
STDMETHODIMP CFXenonConsole::OnStdNotify(_XboxDebugEventType eEventCode, IXboxEventInfo *pEventInfo)
{
	XBOX_EVENT_INFO Info;
	
	if(SUCCEEDED(pEventInfo->get_Info(&Info)))
	{
		switch(eEventCode)
		{
		case eXboxDebugEventType::AssertionFailed:
		case eXboxDebugEventType::AssertionFailedEx:
		case eXboxDebugEventType::RIP:
		case eXboxDebugEventType::Exception:
			{
				if((TxtCallback || CrashCallback) && GetTickCount() - LastExceptionTick >= 10000 && !(eEventCode == eXboxDebugEventType::Exception && Info.Code == MS_VC_EXCEPTION))
				{
					if(this->TxtCallback)
					{
						this->TxtCallback(L"A crash has been detected and UnrealConsole will now attempt to parse its symbols and generate a mini-dump. Please be patient.\r\n");
					}

					// update this here in case we hit an early exit condition as well as at the end
					LastExceptionTick = GetTickCount();

					CComBSTR ProcName;
					std::wstring ProcNameWStr;
					
					if(SUCCEEDED(get_RunningProcessName(&ProcName)))
					{
						ProcNameWStr = (wchar_t*)ProcName.m_str;

						if(ProcNameWStr.find(L"XeDebug") != std::wstring::npos && !(CrashFilter & FConsoleSupport::CRF_Debug))
						{
							break;
						}
						else if(ProcNameWStr.find(L"XeRelease") != std::wstring::npos)
						{
							if(ProcNameWStr.find(L"LTCG") != std::wstring::npos)
							{
								if(!(CrashFilter & FConsoleSupport::CRF_ReleaseForShip))
								{
									break;
								}
							}
							else if(!(CrashFilter & FConsoleSupport::CRF_Release))
							{
								break;
							}
						}
					}

					FCrashThreadData *ThreadData = new FCrashThreadData();
					ThreadData->Console = this;
					ThreadData->CrashFunc = this->CrashCallback;
					ThreadData->TTYFunc = this->TxtCallback;
					ThreadData->ThreadCallback = CFXenonConsole::CrashThreadProc;
					ThreadData->CrashDumpFlags = this->DumpType;

					ThreadData->CallStack.push_back(Info.Address);
					ParseStackFrame(Info.Thread, ThreadData->CallStack);

					char NameBuf[256];
					sprintf_s(NameBuf, 256, "%S", (wchar_t*)TMName.m_str);

					// Load the current xbox into the DM API. It's ok to do this here because this is in the STA and is guaranteed to be thread safe
					DmSetXboxNameNoRegister(NameBuf);

					CComPtr<IXboxModules> Modules;
					DebugTgt->get_Modules(&Modules);

					CComPtr<IXboxModule> CurModule;
					LONG NumModules = 0;
					Modules->get_Count(&NumModules);

					_XboxConsoleType TargetType;
					this->ConsolePtr->get_ConsoleType(&TargetType);

					for(LONG i = 0; i < NumModules; ++i)
					{
						PDBEntry SymbolEntry;
						ZeroMemory(&SymbolEntry, sizeof(SymbolEntry));

						if(SUCCEEDED(Modules->get_Item(i, &CurModule)))
						{
							XBOX_MODULE_INFO ModuleInfo;
							if(SUCCEEDED(CurModule->get_ModuleInfo(&ModuleInfo)))
							{
								std::wstring ModuleName = (wchar_t*)ModuleInfo.Name;

								SysFreeString(ModuleInfo.Name);
								SysFreeString(ModuleInfo.FullName);

								// for whatever reason symbols currently do not exist on the XDK symbol server for these modules and it causes issues
								if(ModuleName == L"ximecore.xex" || (TargetType != eXboxConsoleType::DevelopmentKit && ModuleName == L"xboxkrnl.exe"))
								{
									CurModule.Release();
									continue;
								}

								// NOTE: This double cast gets rid of potential 64-bit compatibility issues
								if(SUCCEEDED(DmFindPdbSignature((PVOID)(unsigned __int64)ModuleInfo.BaseAddress, &SymbolEntry.PDBSig)))
								{
									SymbolEntry.BaseAddress = ModuleInfo.BaseAddress;
									SymbolEntry.Size = ModuleInfo.Size;

									ThreadData->ModulePDBList.push_back(SymbolEntry);
								}
							}
						}

						CurModule.Release();
					}


					DWORD ThreadId;
					HANDLE ThreadHandle = CreateThread(NULL, 0, CFXenonConsole::ThreadMain, ThreadData, 0, &ThreadId);

					if(!ThreadHandle)
					{
						OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::OnStdNotify(): Failed to create callstack parsing thread!\n");
					}
					else
					{
						SetThreadName(ThreadId, "CrashThreadProc");
						ThreadHandles.push_back(ThreadHandle);
					}

					// put this here again to update it as it's possible all of this parsing took a while
					LastExceptionTick = GetTickCount();
				}

				break;
			}
		case eXboxDebugEventType::ExecStateChange:
			{
				//OutputDebugStringW(L"State Change: ");
				//OutputDebugStringW(EXECSTATE[Info.ExecState]);
				//OutputDebugStringW(L"\r\n");

				InterlockedExchange(&ExecState, Info.ExecState);

				break;
			}
		case eXboxDebugEventType::DebugString:
			{
				OnTTY((wchar_t*)Info.Message);
				break;
			}
		}

		DebugTgt->FreeEventInfo(&Info);
	}

	return S_OK;
}

/**
 * Debug channel output.
 *
 * @param	Source			The name of the source channel.
 * @param	Notification	The debug output.
 */
STDMETHODIMP CFXenonConsole::OnTextNotify(BSTR Source, BSTR Notification)
{
#if _DEBUG
	OutputDebugStringW(Source);
	OutputDebugStringW(L"!");
	OutputDebugStringW(Notification);
#endif

	if(wcscmp((wchar_t*)Source, L"UE_PROFILER") != 0)
	{
		return S_OK;
	}

	// Determine the type of profile...
	wchar_t *FilenameStr = wcsstr((wchar_t*)Notification, L":");
	
	if(FilenameStr == NULL)
	{
		OutputDebugStringW(L"Invalid format of notification string: ");
		OutputDebugStringW((wchar_t*)Notification);
		OutputDebugStringW(L"\n");
		return FALSE;
	}

	wchar_t ProfileType[32];

	wcsncpy_s(ProfileType, 32, (wchar_t*)Notification, wcslen((wchar_t*)Notification) - wcslen(FilenameStr));

	FConsoleSupport::EProfileType ProfType = FConsoleSupport::PT_Invalid;

	// now determine which ProfileType this is based on what the engine has passed into us
	if(wcsncmp(ProfileType, L"RENDER", 6) == 0)
	{
		ProfType = FConsoleSupport::PT_RenderTick;
	}
	else if(wcsncmp(ProfileType, L"GAME", 4) == 0)
	{
		ProfType = FConsoleSupport::PT_GameTick;
	}
	else if(wcsncmp(ProfileType, L"SCRIPT", 6) == 0)
	{
		ProfType = FConsoleSupport::PT_Script;
	}
	else if(wcsncmp(ProfileType, L"MEMORY", 6) == 0)
	{
		ProfType = FConsoleSupport::PT_Memory;
	}
	else if(wcsncmp(ProfileType, L"UE3STATS", 7) == 0)
	{
		ProfType = FConsoleSupport::PT_UE3Stats;
	}
	else if(wcsncmp(ProfileType, L"MEMLEAK", 7) == 0)
	{
		ProfType = FConsoleSupport::PT_MemLeaks;
	}
	else if(wcsncmp(ProfileType, L"FPSCHART", 7) == 0)
	{
		ProfType = FConsoleSupport::PT_FPSCharts;
	}
	else if(wcsncmp(ProfileType, L"BUGIT", 5) == 0)
	{
		ProfType = FConsoleSupport::PT_BugIt;
	}
	else if(wcsncmp(ProfileType, L"MISCFILE", 8) == 0)
	{
		ProfType = FConsoleSupport::PT_MiscFile;
	}




	if(ProfType == FConsoleSupport::PT_Invalid)
	{
		OutputDebugStringW(L"Invalid ProfilerType in notification string: ");
		OutputDebugStringW((wchar_t*)Notification);
		OutputDebugStringW(L"\n");
		return FALSE;
	}

	// Advance the filename pointer beyond the ':'
	FilenameStr += 1;

	// Deal(aka remove) with any 'special' stuff tagged on the front of the filename...
	switch (ProfType)
	{
	case FConsoleSupport::PT_Script:
		FilenameStr += wcslen(L"..\\");
		break;
	case FConsoleSupport::PT_GameTick:
	case FConsoleSupport::PT_RenderTick:
		FilenameStr += wcslen(L"GAME:\\");
		break;
	case FConsoleSupport::PT_Memory:
		FilenameStr += wcslen(L"..\\");
		break;
	case FConsoleSupport::PT_UE3Stats:
		FilenameStr += wcslen(L"..\\");
		break;
	case FConsoleSupport::PT_MemLeaks:
		FilenameStr += wcslen(L"..\\");
		break;
	case FConsoleSupport::PT_FPSCharts:
		FilenameStr += wcslen(L"..\\");
		break;
	case FConsoleSupport::PT_BugIt:
		FilenameStr += wcslen(L"..\\");
		break;
	case FConsoleSupport::PT_MiscFile:
		FilenameStr += wcslen(L"..\\");
		break;
	}

	FProfileThreadData *ThreadData = new FProfileThreadData();
	ThreadData->Console = this;
	ThreadData->TTYFunc = this->TxtCallback;
	ThreadData->FileName = FilenameStr;
	ThreadData->Type = ProfType;
	ThreadData->ThreadCallback = CFXenonConsole::ProfileThreadProc;

	DWORD ThreadId;
	HANDLE ThreadHandle = CreateThread(NULL, 0, CFXenonConsole::ThreadMain, ThreadData, 0, &ThreadId);

	if(!ThreadHandle)
	{
		OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::OnTextNotify(): Failed to create profile copying thread!\n");
	}
	else
	{
		ThreadHandles.push_back(ThreadHandle);
	}

	return S_OK;
}

// IFXenonConsole functions

/**
 * Constructor.
 */
CFXenonConsole::CFXenonConsole()
: bCurrentlyDebugging(VARIANT_FALSE), bIsDefaultConsole(VARIANT_FALSE), EndpointCookie(0), LastExceptionTick(0), CrashFilter(FConsoleSupport::CRF_All),
ExecState(eXboxExecutionState::Stopped), TxtCallback(NULL), CrashCallback(NULL), DumpType(eXboxDumpFlags::Normal)
{
	LoadSymbolHelpers();
}

/**
 * Thread for copying PDB's without hanging the UI.
 *
 * @param	lpParameter		Pointer to extra thread data.
 */
DWORD WINAPI CFXenonConsole::ThreadMain(LPVOID lpParameter)
{
	FThreadData *ThreadData = (FThreadData*)lpParameter;

	if(FAILED(CoInitialize(NULL)) || ThreadData == NULL)
	{
		delete ThreadData;
		return 1;
	}

	DWORD ReturnCode = 1;

	CComPtr<IXboxManager> XenonManager;
	if(SUCCEEDED( XenonManager.CoCreateInstance(__uuidof(XboxManager), NULL, CLSCTX_INPROC_SERVER)))
	{
		CComPtr<IXboxConsoles> Consoles;
		if(SUCCEEDED(XenonManager->get_Consoles(&Consoles)))
		{
			LONG NumConsoles = 0;
			if(SUCCEEDED(Consoles->get_Count(&NumConsoles)))
			{
				for(LONG ConsoleIndex = 0; ConsoleIndex < NumConsoles; ++ConsoleIndex)
				{
					CComBSTR ConsoleName;
					if(FAILED(Consoles->get_Item(ConsoleIndex, &ConsoleName)))
					{
						continue;
					}

					// TMName only gets set once when the object is created so this shouldn't be an issue
					if(ConsoleName == ThreadData->Console->TMName)
					{
						CComPtr<IXboxConsole> Target;
						if(FAILED(XenonManager->OpenConsole(ConsoleName, &Target)))
						{
							ReturnCode = 1;
						}
						else
						{
							ReturnCode = ThreadData->ThreadCallback(Target, ThreadData);
						}

						break;
					}
				}
			}
		}
	}

	CoUninitialize();

	delete ThreadData;

	return ReturnCode;
}

/**
 * Builds the final symbol search path.
 *
 * @param	PDBLocation		The location the pdb's were copied to.
 */
char* CFXenonConsole::BuildSymbolSearchPath(const wchar_t *PDBSymbolPath)
{
	// Create an XboxManager object to let us get the symbol server path.
	CComPtr <IXboxManager> Manager;

	if(FAILED(Manager.CoCreateInstance(__uuidof(XboxManager))))
	{
		OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::BuildSymbolSearchPath(): Could not create xbox manager!\n");
		return NULL;
	}

	// Get the XEDK symbol server path.
	CComBSTR SymbolServerPath = NULL;
	if(FAILED(Manager->get_SystemSymbolServerPath(&SymbolServerPath)))
	{
		OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::BuildSymbolSearchPath(): Failed to retrieve XEDK symbol server path!\n");

		HKEY SymKey;
		bool bSuccess = false;

		if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Xbox\\2.0\\SDK"), 0, KEY_READ | KEY_WRITE | KEY_WOW64_32KEY, &SymKey) == ERROR_SUCCESS)
		{
			const int KEYBUFSIZE = 1024;
			DWORD KeyType;
			wchar_t KeyBuf[KEYBUFSIZE];
			DWORD KeyBufSize = KEYBUFSIZE * sizeof(wchar_t);

			if(RegQueryValueExW(SymKey, L"SymbolServer", NULL, &KeyType, (BYTE*)KeyBuf, &KeyBufSize) == ERROR_SUCCESS)
			{
				if(KeyBufSize / sizeof(wchar_t) == KEYBUFSIZE)
				{
					KeyBuf[KEYBUFSIZE - 1] = 0;
				}

				bSuccess = true;
				SymbolServerPath = KeyBuf;
			}
			else
			{
				// Get the XEDK environment variable.
				wchar_t* XedkDir;
				size_t XedkDirSize;
				errno_t Err = _wdupenv_s(&XedkDir, &XedkDirSize, L"xedk");

				if(!Err && XedkDir)
				{
					bSuccess = true;

					SymbolServerPath = L"SRV*";
					SymbolServerPath += XedkDir;
					SymbolServerPath += L"\\bin\\xbox\\symsrv";

					free(XedkDir);
					XedkDir = NULL;

					if(RegSetValueExW(SymKey, L"SymbolServer", 0, REG_SZ, (BYTE*)SymbolServerPath.m_str, SymbolServerPath.Length() * sizeof(wchar_t)) != ERROR_SUCCESS)
					{
						OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::BuildSymbolSearchPath(): Failed to create Symbol Server registry key!\n");
					}
				}
			}

			RegCloseKey(SymKey);
		}

		if(!bSuccess)
		{
			return NULL;
		}
	}

	// Convert the symbol server path from wide characters to char.
	char MBCSSymbolServerPath[MAX_PATH];
	sprintf_s(MBCSSymbolServerPath, "%S", SymbolServerPath);

	// Now build up a complete symbol search path to give to DbgHelp.

	// Add the XEDK symbol server to the symbol search path.
	std::string FullSearchPath = MBCSSymbolServerPath;

	sprintf_s(MBCSSymbolServerPath, ";%S", PDBSymbolPath);
	FullSearchPath += MBCSSymbolServerPath;

	// Add the current directory to the search path.
	//FullSearchPath += std::string( ";." );

	return _strdup(FullSearchPath.c_str());
}

/**
 * Thread for copying PDB's without hanging the UI.
 *
 * @param	Target				The target to handle the crash for.
 * @param	BaseThreadData		Information about the crash.
 */
DWORD WINAPI CFXenonConsole::CrashThreadProc(CComPtr<IXboxConsole> Target, FThreadData *BaseThreadData)
{
	FCrashThreadData *ThreadData = (FCrashThreadData*)BaseThreadData;
	std::wstring SymbolPath;
	std::wstring LocalExe;
	
	CComBSTR ConsoleName;
	if(FAILED(Target->get_Name(&ConsoleName)))
	{
		ConsoleName = L"NA";
	}

	if(FAILED(InternalRetrievePdbFile(Target, ThreadData, SymbolPath, LocalExe, L"")))
	{
		OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::CrashThreadProc(): Failed to copy debug symbols from the target console!\n");
		return 1;
	}

	if(SymbolPath.size() == 0)
	{
		SymbolPath = L"Xenon\\Lib\\XeDebug;Xenon\\Lib\\XeRelease;Xenon\\Lib\\XeReleaseLTCG;Xenon\\Lib\\XeReleaseLTCG-DebugConsole\n";
	}

	char *Buf = BuildSymbolSearchPath(SymbolPath.c_str());

	if(!Buf)
	{
		OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::CrashThreadProc(): Failed to build symbol server path!\n");

		ShowTargetErrorMessage(Target, L"Unable to parse call stack", L"Could not build symbol server search path for crashed target. Please ensure that the HKEY_LOCAL_MACHINE\\SOFTWARE\\Wow6432Node\\Microsoft\\Xbox\\2.0\\SDK\\SymbolServer registry key exists and is valid!");

		return 1;
	}

	DWORD RetCode = 0;

	EnterCriticalSection(&SymbolCS);

	if(SymInitialize(ThreadData->Console, Buf, FALSE))
	{
		free(Buf);
		Buf = NULL;

		std::vector<CComPtr<IDiaSession> > LoadedSymbols;

		if(FAILED(LoadSymbolsForModules(ThreadData, LoadedSymbols)))
		{
			SymCleanup(ThreadData->Console);
			LeaveCriticalSection(&SymbolCS);

			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::CrashThreadProc(): Failed to load symbols!\n");

			wchar_t Temp[1024];
			swprintf_s(Temp, 1024, L"Target \'%s\' has crashed but the symbols could not be loaded. Please make sure that you have an internet connection (for the symbol server) and that the game pdb's are in your Binaries\\Lib\\Xenon\\<configuration> directory on the target console.", (wchar_t*)ConsoleName.m_str);

			ShowTargetErrorMessage(Target, L"Unable to parse call stack", Temp);

			return 1;
		}

		std::wstring FinalCallstack;

		for(size_t i = 0; i < ThreadData->CallStack.size(); ++i)
		{
			FinalCallstack += ResolveAddressToString(ThreadData, LoadedSymbols, ThreadData->CallStack[i]);
		}

		SymCleanup(ThreadData->Console);

		LeaveCriticalSection(&SymbolCS);

		std::wstring DumpPath = L"Temp\\";
		DumpPath += (wchar_t*)ConsoleName.m_str;
		DumpPath += L"\\Dumps";

		_wmkdir(DumpPath.c_str());

		DumpPath += L"\\UnrealConsole_";

		SYSTEMTIME Time;
		ZeroMemory(&Time, sizeof(Time));
		GetLocalTime(&Time);

		wchar_t DumpFileName[MAX_PATH];
		swprintf_s(DumpFileName, MAX_PATH, L"%s%d-%d-%d_%d-%d-%d.dmp", DumpPath.c_str(), Time.wYear, Time.wMonth, Time.wDay, Time.wHour, Time.wMinute, Time.wSecond);

		CComPtr<IXboxDebugTarget> DbgTgt;
		CComBSTR DumpFileNameBSTR(DumpFileName);
		HRESULT hr = S_OK;

		if(SUCCEEDED(hr = Target->get_DebugTarget(&DbgTgt)))
		{
			if(SUCCEEDED(hr = DbgTgt->ConnectAsDebugger(L"DumpDebugger", eXboxDebugConnectFlags::Force)))
			{
				hr = DbgTgt->WriteDump(DumpFileNameBSTR, ThreadData->CrashDumpFlags);

				if(ThreadData->TTYFunc)
				{
					wchar_t Temp[1024];
					swprintf_s(Temp, 1024, L"Saved dump file to %s.\r\n", DumpFileName);
					ThreadData->TTYFunc(Temp);
				}

				DbgTgt->DisconnectAsDebugger();
			}
		}

		if(FAILED(hr))
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::CrashThreadProc(): Failed to initiate crash dump!\n");

			if(ThreadData->TTYFunc)
			{
				ThreadData->TTYFunc(L"Failed to initiate crash dump!\r\n");
			}
		}

		if(ThreadData->CrashFunc)
		{
			ThreadData->CrashFunc(FinalCallstack.c_str(), DumpFileName);
		}
		else if(ThreadData->TTYFunc)
		{
			ThreadData->TTYFunc(L"\r\n");
			ThreadData->TTYFunc(FinalCallstack.c_str());
		}
	}
	else
	{
		LeaveCriticalSection(&SymbolCS);

		RetCode = 1;

		free(Buf);
		Buf = NULL;

		OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::CrashThreadProc(): Failed to initialize debug symbols!\n");

		ShowTargetErrorMessage(Target, L"Unable to parse call stack", L"Failed to initialize the dbghelp API!");
	}

	return RetCode;
}

/**
 * Loads the symbols for each pdb.
 *
 * @param	Context				Information about the context of the crash.
 * @param	OutLoadedModules	Receives the symbol sessions.
 */
HRESULT CFXenonConsole::LoadSymbolsForModules(FCrashThreadData *ThreadData, std::vector<CComPtr<IDiaSession> > &OutLoadedModules)
{
	// Get the XEDK environment variable.
	wchar_t* XedkDir;
	size_t XedkDirSize;
	errno_t Err = _wdupenv_s(&XedkDir, &XedkDirSize, L"xedk");
	if(Err || !XedkDir)
	{
		OutputDebugStringW(L"Couldn't read xedk environment variable.\n");
		return E_FAIL;
	}

	HRESULT hr = S_OK;
	std::wstring DiaPath = XedkDir;
	DiaPath += L"\\bin\\win32\\msdia80.dll";
	char ResultPath[MAX_PATH];

	free(XedkDir);
	XedkDir = NULL;

	for(size_t i = 0; i < ThreadData->ModulePDBList.size(); ++i)
	{
		CComPtr<IDiaDataSource> DiaSource;

		if(FAILED(hr = NoRegCoCreate(DiaPath.c_str(), CLSID_DiaSource, IID_IDiaDataSource, (void**)&DiaSource)))
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolsForModules(): Failed to create IDiaDataSource!\n");
			return hr;
		}

			if(!SymFindFileInPath(ThreadData->Console, NULL, ThreadData->ModulePDBList[i].PDBSig.Path, &ThreadData->ModulePDBList[i].PDBSig.Guid, ThreadData->ModulePDBList[i].PDBSig.Age, 0, SSRVOPT_GUIDPTR, ResultPath, NULL, NULL))
			{
				OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolsForModules(): Could not find symbols in search path!\n");
				return E_FAIL;
			}

			CComBSTR BstrResultPath(ResultPath);
			CComBSTR BstrOriginalPath(ThreadData->ModulePDBList[i].PDBSig.Path);

			if(FAILED(hr = DiaSource->loadAndValidateDataFromPdb(BstrResultPath, &ThreadData->ModulePDBList[i].PDBSig.Guid, 0, ThreadData->ModulePDBList[i].PDBSig.Age)) &&
				FAILED(DiaSource->loadAndValidateDataFromPdb(BstrOriginalPath, &ThreadData->ModulePDBList[i].PDBSig.Guid, 0, ThreadData->ModulePDBList[i].PDBSig.Age)))
			{
				OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolsForModules(): Failed to load and validate pdb!\n");
				return hr;
			}

		CComPtr<IDiaSession> Session;

		if(FAILED(hr = DiaSource->openSession(&Session)))
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolsForModules(): Failed to open DIA session to pdb!\n");
			return hr;
		}

		if(FAILED(hr = Session->put_loadAddress(ThreadData->ModulePDBList[i].BaseAddress)))
		{
			OutputDebugStringW(L"(XeCOMClasses.dll) CFXenonConsole::LoadSymbolsForModules(): Failed to set base address for DIA session!\n");
			return hr;
		}

		OutLoadedModules.push_back(Session);
	}

	return S_OK;
}

/**
 * Shows an error message box to the user with info from the supplied target.
 *
 * @param	Target		The target that the problem occured with.
 * @param	Caption		The caption for the message box.
 * @param	Message		The message to display.
 */
void CFXenonConsole::ShowTargetErrorMessage(CComPtr<IXboxConsole> Target, const wchar_t *Caption, const wchar_t *Message)
{
	wchar_t FullCaption[1024];
	CComBSTR ConsoleName;

	if(FAILED(Target->get_Name(&ConsoleName)))
	{
		ConsoleName = L"n/a";
	}

	swprintf_s(FullCaption, 1024, L"XeTools.dll (%s): %s", (wchar_t*)ConsoleName.m_str, Caption);

	MessageBoxW(NULL, Message, FullCaption, MB_OK);
}

/**
 * Thread callback for processing a profile.
 *
 * @param	lpParameter		Pointer to context information.
 */
DWORD WINAPI CFXenonConsole::ProfileThreadProc(CComPtr<IXboxConsole> Target, FThreadData *BaseThreadData)
{
	FProfileThreadData *ThreadData = (FProfileThreadData*)BaseThreadData;

	bool bRequiresPDB = false;

	// Form the full remote name and local name
	std::wstring RemoteDir;		// The remote directory
	std::wstring LocalDir;		// The local directory

	if(!Target)
	{
		return 1;
	}

	if(FAILED(GetRemoteExecutableDirectory(Target, RemoteDir)))
	{
		return 1;
	}

	if(FAILED(GetLocalExecutableDirectory(LocalDir)))
	{
		return 1;
	}

	// Set the Local directory, and create it if it is not present
	LocalDir += L"\\Temp";
	_wmkdir(LocalDir.c_str());
	LocalDir += L"\\";

	CComBSTR TempBstr;
	Target->get_Name(&TempBstr);

	LocalDir += (wchar_t*)TempBstr.m_str;

	_wmkdir(LocalDir.c_str());
	LocalDir += L"\\Profiling\\";
	_wmkdir(LocalDir.c_str());

	switch (ThreadData->Type)
	{
	case FConsoleSupport::PT_Script:
		LocalDir += L"Script\\";
		break;
	case FConsoleSupport::PT_GameTick:
	case FConsoleSupport::PT_RenderTick:
		bRequiresPDB = true;
		LocalDir += L"Trace\\";
		break;
	case FConsoleSupport::PT_Memory:
		bRequiresPDB = true;
		LocalDir += L"Memory\\";
		break;
	case FConsoleSupport::PT_UE3Stats:
		LocalDir += L"UE3Stats\\";
		break; 
	case FConsoleSupport::PT_MemLeaks:
		LocalDir += L"MemLeaks\\";
		break; 
	case FConsoleSupport::PT_FPSCharts:
		LocalDir += L"FPSCharts\\";
		break; 
	case FConsoleSupport::PT_BugIt:
		LocalDir += L"BugIt\\";
		break; 
	case FConsoleSupport::PT_MiscFile:
		LocalDir += L"MiscFiles\\";
		break; 
	}

	_wmkdir(LocalDir.c_str());

	std::wstring LocalFileName = ThreadData->FileName;
	std::wstring::size_type NameIndex = LocalFileName.find_last_of(L'\\');

	std::wstring LocalSubDirPath = L"";

	std::wstring LocalSubDirName = L"\\";
	if(NameIndex != std::wstring::npos)
	{
		LocalFileName = LocalFileName.substr(NameIndex + 1);

		std::wstring::size_type DirIndex = ThreadData->FileName.rfind( L'\\', (NameIndex-1) );
		if( DirIndex != std::wstring::npos )
		{
			LocalSubDirName = ThreadData->FileName.substr( DirIndex, (NameIndex-DirIndex) );

			LocalDir += LocalSubDirName;
			LocalDir += L'\\';
			LocalSubDirPath = LocalDir; // save this off so we can copy pdbs to it
			_wmkdir(LocalDir.c_str());
		}
	}

	// Store off the local path to use for the local file

	LocalDir += LocalFileName;

	RemoteDir += L'\\';
	RemoteDir += ThreadData->FileName;

	CComBSTR LocalPathBSTR(LocalDir.c_str());
	CComBSTR RemotePathBSTR(L"xe:\\");
	RemotePathBSTR += RemoteDir.c_str() + 3;

	if(ThreadData->TTYFunc)
	{
		std::wstring OutputStr(L"Copying \'");
		OutputStr += (wchar_t*)RemotePathBSTR.m_str;
		OutputStr += L" to ";
		OutputStr += (wchar_t*)LocalPathBSTR.m_str;
		OutputStr += L"\'\r\n";

		ThreadData->TTYFunc(OutputStr.c_str());
	}

	HRESULT Temp;

	// okie here we want to copy the file we just got over the network to some directory
	// and then also copy the "cached"/"local" version of the .pdb to that same location so there is a nice little package
	// which will persist through time


	// for anything that requires the PDB we also need to bring along the EXE
	if(bRequiresPDB == true)
	{
		// EXE Copy it (sort of crazy here.  This should all be refactored as the internalpdb seems to copy file also)
		CComBSTR RemoteExeBSTR;
		std::wstring RemoteExe;

		XBOX_PROCESS_INFO Info;

		if(SUCCEEDED(Target->get_RunningProcessInfo(&Info)))
		{
			RemoteExeBSTR.Attach(Info.ProgramName);
			RemoteExe = (wchar_t*)RemoteExeBSTR.m_str;
			RemoteExe.replace(0, 2, L"xe:");
			RemoteExe.replace(RemoteExe.length() - 4, 4, L".exe");
			RemoteExeBSTR = CComBSTR(RemoteExe.c_str());
		}

		std::wstring XexName;
		std::wstring::size_type Offset = RemoteExe.find_last_of(L'\\');

		if(Offset != std::wstring::npos)
		{
			XexName = RemoteExe.substr(Offset + 1);
		}
		else
		{
			OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): Failed to retrieve XeX name!\n"));
			return E_FAIL;
		}

		CComBSTR ActualRemoteXeXFile(RemoteExeBSTR);
		CComBSTR ActualLocalDir((LocalSubDirPath + XexName).c_str());

		if(ThreadData->TTYFunc)
		{
			std::wstring OutputStr(L"Copying \'");
			OutputStr += (wchar_t*)ActualRemoteXeXFile.m_str;
			OutputStr += L" to ";
			OutputStr += ActualLocalDir;
			OutputStr += L"\'\r\n";

			ThreadData->TTYFunc(OutputStr.c_str());
		}

		if( FAILED(Target->ReceiveFile(ActualLocalDir, ActualRemoteXeXFile) ))
		{
			if(ThreadData->TTYFunc)
			{
				CComBSTR OutputStr(L"Failed to copy \'");
				OutputStr += ActualRemoteXeXFile;
				OutputStr += L"\'\r\n";

				ThreadData->TTYFunc((wchar_t*)OutputStr.m_str);
			}
		}
	}

	// PDB  Copy it
	if(SUCCEEDED(Temp = Target->ReceiveFile(LocalPathBSTR, RemotePathBSTR)))
	{
		if(bRequiresPDB == true)
		{
			std::wstring SymbolPath;
			std::wstring LocalExe;

			if(FAILED(InternalRetrievePdbFile(Target, ThreadData, SymbolPath, LocalExe, L"")))
			{
				return 1;
			}
            
            // these copy the pdb files to the local sub dir so we have a "package" we can pass around
            // @TODO:  don't re copy from the xenon  as that is semi slow.  (but it works now)
			if(FAILED(InternalRetrievePdbFile(Target, ThreadData, SymbolPath, LocalExe, LocalSubDirPath)))
			{
				return 1;
			}
		}
	}
	else if(ThreadData->TTYFunc)
	{
		std::wstring OutputStr(L"Failed to copy \'");
		OutputStr += RemoteDir;
		OutputStr += L"\'\r\n";

		ThreadData->TTYFunc(OutputStr.c_str());

		return 1;
	}


	PROCESS_INFORMATION ProcInfo;
	STARTUPINFOW StartInfo;

	ZeroMemory(&StartInfo, sizeof(StartInfo));
	StartInfo.cb = sizeof(StartInfo);


	wchar_t CmdLine[_MAX_PATH];
	wsprintf(CmdLine, L"\"%s\"", LocalDir.c_str());

	const int MAX_OUTPUT_BUF = 512;
	wchar_t OutputBuf[MAX_OUTPUT_BUF];

	switch(ThreadData->Type)
	{
	case FConsoleSupport::PT_GameTick:
	case FConsoleSupport::PT_RenderTick:
		{
			// look for the DLL in the installed path
			wchar_t XdkPath[_MAX_PATH];
			DWORD Error = GetEnvironmentVariableW(L"XEDK", XdkPath, _MAX_PATH);

			if(Error > 0)
			{
				wcscat_s(XdkPath, _MAX_PATH, L"\\Bin\\Win32\\PIX.exe");

				wsprintf(OutputBuf, L"%s %s", XdkPath, CmdLine);

				if(CreateProcessW(NULL, OutputBuf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
				{
					CloseHandle(ProcInfo.hThread);
					CloseHandle(ProcInfo.hProcess);

					wsprintf(OutputBuf, L"Profile opened with \'%s %s\'\r\n", XdkPath, CmdLine);

					if(ThreadData->TTYFunc)
					{
						ThreadData->TTYFunc(OutputBuf);
					}
				}
				else
				{
					if(ThreadData->TTYFunc)
					{
						ThreadData->TTYFunc(L"Could not start PIX!\r\n");
					}

					return 1;
				}
			}
			else
			{
				return 1;
			}

			break;
		}
		// refactor this to be function
	case FConsoleSupport::PT_Script:
		{
			wsprintf(OutputBuf, L"UeScriptProfiler.exe %s", CmdLine);

			if(CreateProcessW(NULL, OutputBuf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
			{
				CloseHandle(ProcInfo.hThread);
				CloseHandle(ProcInfo.hProcess);

				wsprintf(OutputBuf, L"Profile opened with \'UeScriptProfiler.exe %s\'\r\n", CmdLine);

				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(OutputBuf);
				}
			}
			else
			{
				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(L"Could not start UeScriptProfiler.exe!\r\n");
				}

				return 1;
			}
			break;
		}
	case FConsoleSupport::PT_Memory:
		{
			wsprintf(OutputBuf, L"MemoryProfiler2.exe %s", CmdLine);

			if(CreateProcessW(NULL, OutputBuf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
			{
				CloseHandle(ProcInfo.hThread);
				CloseHandle(ProcInfo.hProcess);

				wsprintf(OutputBuf, L"Profile opened with \'MemoryProfiler2.exe %s\'\r\n", CmdLine);

				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(OutputBuf);
				}
			}
			else
			{
				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(L"Could not start MemoryProfiler2.exe!\r\n");
				}

				return 1;
			}
			break;
		}
	case FConsoleSupport::PT_UE3Stats:
		{
			wsprintf(OutputBuf, L"StatsViewer.exe %s", CmdLine);

			if(CreateProcessW(NULL, OutputBuf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
			{
				CloseHandle(ProcInfo.hThread);
				CloseHandle(ProcInfo.hProcess);

				wsprintf(OutputBuf, L"Profile opened with \'StatsViewer.exe %s\'\r\n", CmdLine);

				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(OutputBuf);
				}
			}
			else
			{
				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(L"Could not start StatsViewer.exe!\r\n");
				}

				return 1;
			}
			break;
		}
	case FConsoleSupport::PT_MemLeaks:
		{
			wsprintf(OutputBuf, L"Notepad.exe %s", CmdLine);

			if(CreateProcessW(NULL, OutputBuf, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
			{
				CloseHandle(ProcInfo.hThread);
				CloseHandle(ProcInfo.hProcess);

				wsprintf(OutputBuf, L"Profile opened with \'Notepad.exe %s\'\r\n", CmdLine);

				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(OutputBuf);
				}
			}
			else
			{
				if(ThreadData->TTYFunc)
				{
					ThreadData->TTYFunc(L"Could not start Notepad.exe!\r\n");
				}

				return 1;
			}
			break;
		}
	case FConsoleSupport::PT_FPSCharts:
		{
			// do nothing atm they just want them copied
			break;
		}
	case FConsoleSupport::PT_BugIt:
		{
			// do nothing atm they just want them copied
			break;
		}
	case FConsoleSupport::PT_MiscFile:
		{
			// do nothing atm they just want them copied
			break;
		}

	}


	return 0;
}

/**
 * Initializes the console object.
 *
 * @param	Console			A pointer to the console's interface.
 * @param	TargetMgrName	The name of the target as it's known to the target manager (xbox neighborhood).
 * @Param	bIsDefault		True if the target is the default console.
 */
STDMETHODIMP CFXenonConsole::Initialize(IXboxConsole *Console, BSTR TargetMgrName, VARIANT_BOOL bIsDefault)
{
	if(!ConsolePtr)
	{
		ConsolePtr = Console;
		ConsolePtr->get_DebugTarget(&DebugTgt);

		this->bIsDefaultConsole = bIsDefault;
	}

	InterlockedExchange(&ExecState, eXboxExecutionState::Stopped);
	this->TMName = TargetMgrName;

	return S_OK;
}

/**
 * Connects a debugger to the console to receive debug event notifications.
 *
 * @param	DebuggerName	The name of the debugger being attached.
 */
STDMETHODIMP CFXenonConsole::ConnectDebugger(BSTR DebuggerName)
{
	if(!DebugTgt || !ConsolePtr)
	{
		return E_FAIL;
	}

	if(bCurrentlyDebugging == VARIANT_TRUE)
	{
		return S_FALSE;
	}

	HRESULT Result = DebugTgt->ConnectAsDebugger(DebuggerName, eXboxDebugConnectFlags::Force);

	if(SUCCEEDED(Result))
	{
		bCurrentlyDebugging = VARIANT_TRUE;

		InterlockedExchange(&ExecState, eXboxExecutionState::Pending);

		ATLASSERT(SUCCEEDED(Advise()));
	}

	return Result;
}

/**
 * Stops the console from receiving debug event notifications.
 */
STDMETHODIMP CFXenonConsole::DisconnectDebugger()
{
	bCurrentlyDebugging = VARIANT_FALSE;

	if(!DebugTgt)
	{
		return E_FAIL;
	}

	if(FAILED(UnAdvise()))
	{
		OutputDebugStringW(L"Failed UnAdvise() in CFXenonConsole::DisconnectDebugger()\n");
	}

	InterlockedExchange(&ExecState, eXboxExecutionState::Stopped);

	return DebugTgt->DisconnectAsDebugger();
}

/**
 * Retrieves the actual name of the console if it exists otherwise it returns the target manager name for it.
 *
 * @param	OutName		Receives the console's name.
 */
STDMETHODIMP CFXenonConsole::get_Name(BSTR *OutName)
{
	if(CachedName.Length() == 0)
	{
		if(!ConsolePtr)
		{
			return E_FAIL;
		}

		HRESULT Result = ConsolePtr->get_Name(&CachedName);

		if(SUCCEEDED(Result))
		{
			*OutName = SysAllocString(CachedName);
		}
		else
		{
			*OutName = SysAllocString(TMName);
		}
	}
	else
	{
		*OutName = SysAllocString(CachedName);
	}

	return S_OK;
}

/**
 * Retrieves the name of the currently running process on the console.
 *
 * @param	OutName		Receives the process name.
 */
STDMETHODIMP CFXenonConsole::get_RunningProcessName(BSTR *OutName)
{
	*OutName = NULL;

	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	XBOX_PROCESS_INFO Info;
	HRESULT Result = ConsolePtr->get_RunningProcessInfo(&Info);

	if(SUCCEEDED(Result))
	{
		CComBSTR ProcName;
		ProcName.Attach(Info.ProgramName);
		*OutName = ProcName.Copy();
	}

	return Result;
}

/**
 * Retrieves the title IP address of the console and converts it into little endian.
 *
 * @param	OutIPAddress	Receives the title IP address.
 */
STDMETHODIMP CFXenonConsole::get_IPAddressTitle(DWORD *OutIPAddress)
{
	*OutIPAddress = 0;

	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	HRESULT Result = ConsolePtr->get_IPAddressTitle(OutIPAddress);

	if(SUCCEEDED(Result))
	{
		// convert from big endian to little endian
		*OutIPAddress = ntohl(*OutIPAddress);
	}

	return Result;
}

/**
 * Returns whether or not the console has a debugger attached.
 *
 * @param	OutValue	Receives the value indicating whether or not a debugger is attached.
 */
STDMETHODIMP CFXenonConsole::get_IsDebugging(VARIANT_BOOL *OutValue)
{
	*OutValue = bCurrentlyDebugging;

	return S_OK;
}

/**
 * Creates a directory on the console.
 *
 * @param	DirectoryName	The name of the directory to be created.
 */
STDMETHODIMP CFXenonConsole::MakeDirectory(BSTR DirectoryName)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	return ConsolePtr->MakeDirectory(DirectoryName);
}

/**
 * Retrieves a pointer to a file on the console.
 *
 * @param	FilePath	The name of the file.
 * @param	OutFile		Receives the pointer to the file.
 */
STDMETHODIMP CFXenonConsole::GetFileObject(BSTR FilePath, IXboxFile **OutFile)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	return ConsolePtr->GetFileObject(FilePath, OutFile);
}

/**
 * Copies a file to the console.
 *
 * @param	LocalName		The name of the file on the local HD.
 * @param	RemoteName		The name of the file on the console's HD.
 */
STDMETHODIMP CFXenonConsole::SendFile(BSTR LocalName, BSTR RemoteName)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	return ConsolePtr->SendFile(LocalName, RemoteName);
}

/**
 * Copies a file from the console to the local HD.
 *
 * @param	LocalName		The name of the file on the local HD.
 * @param	RemoteName		The name of the file on the console's HD.
 */
STDMETHODIMP CFXenonConsole::ReceiveFile(BSTR LocalName, BSTR RemoteName)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	return ConsolePtr->ReceiveFile(LocalName, RemoteName);
}

/**
 * Reboots the console.
 *
 * @param	Name				The name of the file to load upon reboot.
 * @param	MediaDirectory		The directory containing assets needed to run.
 * @param	CmdLine				The command line for the file that will be executed on startup.
 * @param	Flags				Flags controlling reboot and startup.
 */
STDMETHODIMP CFXenonConsole::Reboot(BSTR Name, BSTR MediaDirectory, BSTR CmdLine, _XboxRebootFlags Flags)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	LONG CurState = InterlockedCompareExchange(&ExecState, 0, 0);

	InterlockedExchange(&ExecState, eXboxExecutionState::Rebooting);

	HRESULT Result = ConsolePtr->Reboot(Name, MediaDirectory, CmdLine, Flags);

	if(FAILED(Result))
	{
		InterlockedExchange(&ExecState, CurState);
	}

	return Result;
}

/**
 * Gets a pointer to the list of threads on the console.
 *
 * @param	OutThreads		Receives a pointer to the list of threads.
 */
STDMETHODIMP CFXenonConsole::get_Threads(IXboxThreads **OutThreads)
{
	OutThreads = NULL;

	if(!DebugTgt)
	{
		return E_FAIL;
	}

	return DebugTgt->get_Threads(OutThreads);
}

/**
 * Takes a screen shot of the console's framebuffer and copies it to the local HD.
 *
 * @param	FileName		The name of the screen shot on the local HD.
 */
STDMETHODIMP CFXenonConsole::ScreenShot(BSTR FileName)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	return ConsolePtr->ScreenShot(FileName);
}

/**
 * Sends a command to the game currently running on the console.
 *
 * @param	Command)		The command.
 */
STDMETHODIMP CFXenonConsole::SendCommand(BSTR Command)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	DWORD Connection;
	HRESULT Result = E_FAIL;

	CComBSTR Channel(L"UNREAL");

	if(SUCCEEDED(ConsolePtr->OpenConnection(Channel, &Connection)))
	{
		CComBSTR Response;
		Result = ConsolePtr->SendTextCommand(Connection, Command, &Response);

		ConsolePtr->CloseConnection(Connection);
	}

	return Result;
}

/**
 * Registers the debug notification event handlers.
 */
STDMETHODIMP CFXenonConsole::Advise()
{
	if(!ConsolePtr || Container || ConnectionPt)
	{
		return E_FAIL;
	}

	// We need an IUnknown for ourself.
	CComPtr<IUnknown> Unk;
	HRESULT hr = QueryInterface(__uuidof(IUnknown), (void**)&Unk);
	
	if(FAILED(hr))
	{
		return hr;
	}

	// Get the IConnectionPointContainer.
	Container = ConsolePtr;
	
	if(!Container)
	{
		return E_NOINTERFACE;
	}

	// Find the connection point for IXboxEvents.
	hr = Container->FindConnectionPoint(__uuidof(XboxEvents), &ConnectionPt);

	if(FAILED(hr))
	{
		return hr;
	}

	// Ask the connection point to advise us.
	hr = ConnectionPt->Advise(Unk, &EndpointCookie);

	return hr;
}

/**
 * Unregisters the debug notification event handlers.
 */
STDMETHODIMP CFXenonConsole::UnAdvise()
{
	if(!Container)
	{
		return E_NOINTERFACE;
	}

	if(!ConnectionPt)
	{
		return E_NOINTERFACE;
	}

	HRESULT hr = ConnectionPt->Unadvise(EndpointCookie);

	ConnectionPt.Release();
	Container.Release();

	return hr;
}

/**
 * Gets a value indicating whether or not the console is the default console.
 *
 * @param	bOutIsDefault		Receives the value indicating whether or not the console is the default.
 */
STDMETHODIMP CFXenonConsole::get_IsDefault(VARIANT_BOOL *bOutIsDefault)
{
	*bOutIsDefault = bIsDefaultConsole;

	return S_OK;
}

/**
 * Gets the state of the console.
 *
 * @param	bOutExecState		Receives the state of the console.
 */
STDMETHODIMP CFXenonConsole::get_ExecState(LONG *bOutExecState)
{
	// according to docs this should be atomic
	*bOutExecState = InterlockedCompareExchange(&ExecState, 0, 0);

	return S_OK;
}

/**
 * Actually generate the callstack from the thread
 *
 * @param Thread Crashed thread, for context
 * @param Buffer Memory to hold the callstack addresses
 */
STDMETHODIMP CFXenonConsole::ParseStackFrame(IXboxThread* Thread, std::vector<DWORD> &Buffer)
{
	// get top of stack
	CComPtr<IXboxStackFrame> StackFrame;

	if(SUCCEEDED(Thread->get_TopOfStack(&StackFrame)))
	{
	bool bReachedTop = false;

	// walk the stack until we reach the top or we are out of space in the buffer
		// sometimes when handling multiple crashes in a row StackFrame can receive a NULL pointer for some crazy reason so we want to check for it
		while(!bReachedTop && StackFrame)
	{
		// get the function address for the stack
		//			XBOX_FUNCTION_INFO FunctionInfo;
		//			StackFrame->get_FunctionInfo(&FunctionInfo);
		DWORD Address;// = FunctionInfo.BeginAddress;
		StackFrame->get_ReturnAddress(&Address);

		if (Address == 0)
		{
			bReachedTop = true;
		}
		else
		{
			Buffer.push_back(Address);

			// go up the stack
			CComPtr<IXboxStackFrame> PrevStackFrame = StackFrame;
			StackFrame.Release();

			if (FAILED(PrevStackFrame->get_NextStackFrame(&StackFrame)))
			{
				bReachedTop = true;
			}
		}
	}
	}
	else
	{
		OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::ParseStackFrame(): Failed to parse call stack!\n"));
	}

	return S_OK;
}

/**
 *	Retrieve the pdb file from the xdk and the local exe name
 *
 *  @param	Target				The target that the crash occured on.
 *  @param	ThreadData			Context information for the thread.
 *	@param	SymbolPath	OUT		The override symbol path to use.
 *	@param	LocalExe	OUT		The local exe name.
 *
 *	@return	bool				true if successful
 */
HRESULT CFXenonConsole::InternalRetrievePdbFile(CComPtr<IXboxConsole> Target, FThreadData *ThreadData, std::wstring& SymbolPath, std::wstring& LocalExe, std::wstring LocalPathOverride )
{
	std::wstring PDBLocalLocation;
	std::wstring AltPdbLoc;

	std::wstring PEPath;			// The local executable
	std::wstring RemotePDBName;	// The remote pdb file
	std::wstring LocalDir;		// The local directory

	CComBSTR RemoteExeBSTR;
	std::wstring RemoteExe;

	XBOX_PROCESS_INFO Info;

	if(SUCCEEDED(Target->get_RunningProcessInfo(&Info)))
	{
		RemoteExeBSTR.Attach(Info.ProgramName);
	}
	else
	{
		OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): Failed to retrieve remote executable name from the xbox!\n"));
		return E_FAIL;
	}

	RemoteExe = (wchar_t*)RemoteExeBSTR.m_str;
	PEPath = (wchar_t*)RemoteExeBSTR.m_str;
	RemotePDBName = (wchar_t*)RemoteExeBSTR.m_str;

	std::wstring XexName;
	std::wstring::size_type Offset = RemoteExe.find_last_of(L'\\');

	if(Offset != std::wstring::npos)
	{
		XexName = RemoteExe.substr(Offset + 1);
	}
	else
	{
		OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): Failed to retrieve XeX name!\n"));
		return E_FAIL;
	}

	// get the location of this DLL
	if(FAILED(GetLocalExecutableDirectory(LocalDir)))
	{
		OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): Failed to retrieve local executable directory!\n"));
		return E_FAIL;
	}

	if( LocalPathOverride != L"")
	{
		PDBLocalLocation = LocalPathOverride;
	}
	else
	{
		// Set the PDBLocal directory, and create it if it is not present
		PDBLocalLocation = LocalDir;
		PDBLocalLocation += L"\\Temp";
		_wmkdir(PDBLocalLocation.c_str());
		PDBLocalLocation += L"\\";
	}

	AltPdbLoc = LocalDir + L"\\Xenon\\Lib\\";

	CComBSTR TargetName;
	if(FAILED(Target->get_Name(&TargetName)))
	{
		OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): Failed to retrieve target name!\n"));
		return E_FAIL;
	}

	if( LocalPathOverride == L"")
	{
		PDBLocalLocation += (wchar_t*)TargetName.m_str;
	}

	_wmkdir(PDBLocalLocation.c_str());

	// make the xbox path into a PC relative path
	Offset = PEPath.rfind(L'\\');
	std::wstring RemoteEXE = PEPath;

	// Replace the 'xex' with 'pdb'
	XexName.replace(XexName.length() - 4, 4, L".pdb");
	RemoteEXE.replace(RemoteEXE.length() - 4, 4, L".exe");
	RemotePDBName.erase(RemotePDBName.length() - XexName.length());
	RemotePDBName += L"Binaries\\Xenon\\Lib\\";
	
	std::wstring Configuration =  XexName.substr(XexName.find(L"-Xe") + 1);
	Configuration.erase(Configuration.length() - 4);
	
	RemotePDBName += Configuration;
	RemotePDBName += L'\\';
	RemotePDBName += XexName;

	AltPdbLoc += Configuration;
	std::wstring AltPdbFile = AltPdbLoc + L'\\' + XexName;

	std::wstring LocalPDBFile = PDBLocalLocation + L'\\' + XexName;
	PEPath = LocalPDBFile;

	CComBSTR LocalPDBFileBSTR(LocalPDBFile.c_str());
	CComBSTR RemotePDBNameBSTR(RemotePDBName.c_str() + 3);
	VARIANT_BOOL bResult;

	// See if the file is out of date (parsing off the 'E:\' on the remote file)
	if(SUCCEEDED(NeedsToCopyFileInternal(Target, LocalPDBFileBSTR, RemotePDBNameBSTR, true, &bResult)))
	{
		if(bResult == VARIANT_TRUE)
		{
			CComBSTR FullDir(L"xe:\\");
			FullDir += RemotePDBNameBSTR;

			if(ThreadData->TTYFunc)
			{
				CComBSTR OutputStr(L"Copying \'");
				OutputStr += FullDir;
				OutputStr += L"\'\r\n";

				ThreadData->TTYFunc((wchar_t*)OutputStr.m_str);
			}

			if(FAILED(Target->ReceiveFile(LocalPDBFileBSTR, FullDir)))
			{
				if(ThreadData->TTYFunc)
				{
					CComBSTR OutputStr(L"Failed to copy \'");
					OutputStr += FullDir;
					OutputStr += L"\'. Make sure the \'Copy files required for symbol lookup\' checkbox is checked in the Console Targets tab of UFE.\r\n";

					ThreadData->TTYFunc((wchar_t*)OutputStr.m_str);
				}

				OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): ReceiveFile() failed!\n"));
				return E_FAIL;
			}
		}
	}
	else
	{
		if(ThreadData->TTYFunc)
		{
			CComBSTR OutputStr(L"Failed to copy \'");
			OutputStr += RemotePDBNameBSTR;
			OutputStr += L"\'\r\n";

			ThreadData->TTYFunc((wchar_t*)OutputStr.m_str);
		}

		OutputDebugString(TEXT("(XeCOMClasses.dll) CFXenonConsole::RetrievePdbFile(): NeedsToCopyFile() on PDB failed!\n"));
		return E_FAIL;
	}

	// replace the .pdb with .exe
	PEPath.replace(PEPath.length() - 4, 4, L".exe");

	SymbolPath = PDBLocalLocation;
	LocalExe = PEPath;

	// make sure the alt path exists
	WIN32_FILE_ATTRIBUTE_DATA AltPathAttrs;
	if(GetFileAttributesExW(AltPdbFile.c_str(), GetFileExInfoStandard, &AltPathAttrs) != 0)
	{
		SymbolPath += ';';
		SymbolPath += AltPdbLoc;
	}

	return S_OK;
}

/**
 *	Get the path of the application running on the PC
 *
 *	@param	LocalExeDir	OUT		The local executable path
 *
 *	@return	bool				true if successful
 */
HRESULT CFXenonConsole::GetLocalExecutableDirectory(std::wstring& LocalExeDir)
{
	const DWORD MODULE_LOC_SIZE = 1024;
	// get the location of this DLL
	wchar_t ModuleLocation[1024];

	DWORD RetVal = GetModuleFileNameW(NULL, ModuleLocation, MODULE_LOC_SIZE);
	if (RetVal == 0)
	{
		return E_FAIL;
	}

	// chop off the \\Blah.exe part
	wchar_t* LastSlash = wcsrchr(ModuleLocation, L'\\');
	if (LastSlash)
	{
		*LastSlash = 0;
	}

	LocalExeDir = ModuleLocation;

	return S_OK;
}

/**
 * Retrieves the directory of the currently running process on the supplied target.
 *
 * @param	Target			The target to get the remote directory from.
 * @param	OutRemoteDir	Receives the remote directory.
 */
HRESULT CFXenonConsole::GetRemoteExecutableDirectory(CComPtr<IXboxConsole> Target, std::wstring &OutRemoteDir)
{
	if(!Target)
	{
		return E_FAIL;
	}

	HRESULT Ret = E_FAIL;
	XBOX_PROCESS_INFO ProcInfo;

	if(SUCCEEDED(Ret = Target->get_RunningProcessInfo(&ProcInfo)))
	{
		OutRemoteDir = (wchar_t*)ProcInfo.ProgramName;
		SysFreeString(ProcInfo.ProgramName);

		std::wstring::size_type Index = OutRemoteDir.find_last_of(L'\\');

		if(Index != std::wstring::npos)
		{
			OutRemoteDir = OutRemoteDir.substr(0, Index);
			Ret = S_OK;
		}
		else
		{
			Ret = E_FAIL;
		}
	}

	return Ret;
}

/**
 * Determines if a file on the local HD and a file on the console are not identical and therefore require syncing.
 *
 * @param	Target				The console to check.
 * @param	SourceFilename		The source file on the local HD.
 * @param	DestFilename		The destination file on the console.
 * @param	bReverse			Reverse the dest and source file names.
 * @param	bOutShouldCopy		Receives a value indicating whether or not a copy needs to occur.
 */
HRESULT CFXenonConsole::NeedsToCopyFileInternal(CComPtr<IXboxConsole> Target, BSTR SourceFilename, BSTR DestFilename, VARIANT_BOOL bReverse, VARIANT_BOOL *bOutShouldCopy)
{
	if(!Target)
	{
		return E_FAIL;
	}

	// make sure the local file exists
	WIN32_FILE_ATTRIBUTE_DATA LocalAttrs;
	if (GetFileAttributesExW(SourceFilename, GetFileExInfoStandard, &LocalAttrs) == 0)
	{
		// If they must be identical, then we would want to copy in this case.
		// (The identical case determines if we need to copy from XDK to local)
		*bOutShouldCopy = bReverse ? VARIANT_TRUE : VARIANT_FALSE;

		return S_OK;
	}

	// check whether destination file exists.
	CComPtr<IXboxFile> XeDestinationFile;
	CComBSTR FullPath(L"xe:\\");
	FullPath += DestFilename;

	if(FAILED(Target->GetFileObject(FullPath, &XeDestinationFile)))
	{
		*bOutShouldCopy = VARIANT_TRUE;
		return S_OK;
	}

	// get xbox file size
	ULONGLONG XboxFileSize;
	XeDestinationFile->get_Size(&XboxFileSize);

	// get local file size
	ULONGLONG LocalFileSize = ((ULONGLONG)LocalAttrs.nFileSizeHigh << 32) | ((ULONGLONG)LocalAttrs.nFileSizeLow);
	if (XboxFileSize != LocalFileSize)
	{
		*bOutShouldCopy = VARIANT_TRUE;
		return S_OK;
	}

	// get the xbox file time
	VARIANT Variant;
	XeDestinationFile->get_ChangeTime(&Variant);

	// convert it to be useful
	SYSTEMTIME XboxSystemTime;
	VariantTimeToSystemTime(Variant.dblVal, &XboxSystemTime);
	
	TIME_ZONE_INFORMATION TimeZoneInfo;
	GetTimeZoneInformation(&TimeZoneInfo);

	SYSTEMTIME XboxUTCTime;
	TzSpecificLocalTimeToSystemTime(&TimeZoneInfo, &XboxSystemTime, &XboxUTCTime);
	
	FILETIME XboxFileTime;
	SystemTimeToFileTime(&XboxUTCTime, &XboxFileTime);

	// compare file times to see if local file is newer
	long FileTimeCmp = CompareFileTime(&XboxFileTime, &LocalAttrs.ftLastWriteTime);
	if ((!bReverse && (FileTimeCmp < 0)) ||
		(bReverse && (FileTimeCmp > 0)))
	{
		*bOutShouldCopy = VARIANT_TRUE;
			return S_OK;
		}

	// if we get here, the files are the same
	*bOutShouldCopy = VARIANT_FALSE;
	return S_OK;
}

/**
 * Determines if a file on the local HD and a file on the console are not identical and therefore require syncing.
 *
 * @param	SourceFilename		The source file on the local HD.
 * @param	DestFilename		The destination file on the console.
 * @param	bReverse			Reverse the dest and source file names.
 * @param	bOutShouldCopy		Receives a value indicating whether or not a copy needs to occur.
 */
STDMETHODIMP CFXenonConsole::NeedsToCopyFile(BSTR SourceFilename, BSTR DestFilename, VARIANT_BOOL bReverse, VARIANT_BOOL *bOutShouldCopy)
{
	return NeedsToCopyFileInternal(ConsolePtr, SourceFilename, DestFilename, bReverse, bOutShouldCopy);
}

/**
 * Resolves an address to a string.
 *
 * @param	ThreadData		Contextual information.
 * @param	LoadedModules	The symbols for the loaded modules.
 * @param	Address			The address of the instruction to be resolved.
 */
std::wstring CFXenonConsole::ResolveAddressToString(FCrashThreadData *ThreadData, std::vector<CComPtr<IDiaSession> > &LoadedModules, DWORD Address)
{
	CComBSTR FuncName;
	CComBSTR FileName;
	bool bFound = false;
	DWORD LineNumber = 0;

	for(size_t i = 0; i < LoadedModules.size() && !bFound; ++i)
	{
		if(Address > ThreadData->ModulePDBList[i].BaseAddress && Address < ThreadData->ModulePDBList[i].BaseAddress + ThreadData->ModulePDBList[i].Size)
		{
			CComPtr<IDiaSession> &Session = LoadedModules[i];
			CComPtr<IDiaSymbol> Symbol;

			if(SUCCEEDED(Session->findSymbolByVA(Address, SymTagFunction, &Symbol)) && Symbol)
			{
				bFound = true;
				Symbol->get_name(&FuncName);

				CComPtr<IDiaEnumLineNumbers> LineEnumerator;

				if(SUCCEEDED(Session->findLinesByVA(Address, 4, &LineEnumerator)))
				{
					// We could loop over all of the source lines that map to this instruction,
					// but there is probably at most one, and if there are multiple source
					// lines we still only want one.
					CComPtr<IDiaLineNumber> Line;
					DWORD celt;

					if(SUCCEEDED(LineEnumerator->Next(1, &Line, &celt)) && celt == 1)
					{
						Line->get_lineNumber(&LineNumber);

						CComPtr<IDiaSourceFile> SrcFile;

						if(SUCCEEDED(Line->get_sourceFile(&SrcFile)))
						{
							SrcFile->get_fileName(&FileName);
						}
					}
				}
			}
		}
	}

	wchar_t Temp[1024];

	if(bFound && FuncName)
	{
		swprintf_s(Temp, 1024, L"%s() [%s:%d]\r\n", (wchar_t*)FuncName.m_str, FileName ? (wchar_t*)FileName.m_str : L"???", LineNumber);
	}
	else
	{
		swprintf_s(Temp, 1024, L"%x() [???:0]\r\n", Address);
	}

	return Temp;
}

/**
 * Calls the TTY output event handler.
 *
 * @param	Txt		The TTY output.
 */
STDMETHODIMP CFXenonConsole::OnTTY(const wchar_t *Txt)
{
	if(TxtCallback)
	{
		TxtCallback(Txt);
	}

	return S_OK;
}

/**
 * Sets the TTY callback pointer. This is a hack around COM and should only be called on in-proc servers.
 *
 *  @param	CallbackPtr		Pointer to the callback function.
 */
STDMETHODIMP CFXenonConsole::SetTTYCallback(unsigned hyper CallbackPtr)
{
	TxtCallback = (TTYCALLBACK)CallbackPtr;

	return S_OK;
}

/**
 * Gets the name the target manager associates with the console.
 *
 *  @param	OutTMName		Receives the name the target manager associates with the console.
 */
STDMETHODIMP CFXenonConsole::get_TargetManagerName(BSTR *OutTMName)
{
	*OutTMName = SysAllocString(TMName);

	return S_OK;
}

/**
 * Sets the crash callback pointer. This is a hack around COM and should only be called on in-proc servers.
 *
 *  @param	CallbackPtr		Pointer to the callback function.
 */
STDMETHODIMP CFXenonConsole::SetCrashCallback(unsigned hyper CallbackPtr)
{
	CrashCallback = (CRASHCALLBACK)CallbackPtr;

	return S_OK;
}

/**
 * Gets the IP address of the debug channel and converts it to little endian.
 *
 *  @param	OutIPAddress		Receives the debug channel IP address.
 */
STDMETHODIMP CFXenonConsole::get_DebugChannelIPAddress(DWORD *OutIPAddress)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	HRESULT Result = ConsolePtr->get_IPAddress(OutIPAddress);

	if(SUCCEEDED(Result))
	{
		*OutIPAddress = ntohl(*OutIPAddress);
	}

	return Result;
}

/**
 * Gets the type of the target console (i.e. debug kit, test kit, etc).
 *
 *  @param	OutTargetType		Receives the type of the console.
 */
STDMETHODIMP CFXenonConsole::get_TargetType(int *OutTargetType)
{
	if(!ConsolePtr)
	{
		return E_FAIL;
	}

	return ConsolePtr->get_ConsoleType((_XboxConsoleType*)OutTargetType);
}

/**
 * Sets the crash report filter for the console.
 *
 * @param	Filter		Flags controlling filtering of crash reports.
 */
STDMETHODIMP CFXenonConsole::put_CrashReportFilter(DWORD Filter)
{
	CrashFilter = Filter;

	return S_OK;
}

/**
 * Sets the dump type for the target.
 *
 * @param	DumpType	The new dump type.
 */
STDMETHODIMP CFXenonConsole::put_DumpType(DWORD DumpType)
{
	this->DumpType = DumpType;
	return S_OK;
}

/**
* Sets the dump type for the target.
*
* @param	OutDumpType		Receives dump type.
*/
STDMETHODIMP CFXenonConsole::get_DumpType(DWORD *OutDumpType)
{
	if(!OutDumpType)
	{
		return E_FAIL;
	}

	*OutDumpType = this->DumpType;
	return S_OK;
}