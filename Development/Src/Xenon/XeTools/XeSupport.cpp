/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

// needed for CoInitializeEx()
#define _WIN32_DCOM
#define _WIN32_WINNT 0x0500 // minimum windows version of windows server 2000

#include <io.h>
#include <direct.h>
#include "XeSupport.h"

#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

// colors for TTY output during sync

#define COLOR_BLACK		0x000000ff
#define COLOR_RED		0xff0000ff

#define SENDFILE_RETRY	10000

struct FSyncThreadInfo
{
	ColoredTTYEventCallbackPtr OutputCallback;
	const wchar_t **DirectoriesToCreate;
	int DirectoriesToCreateSize;
	vector<wstring> SrcFilesToSync;
	vector<wstring> DestFilesToSync;
	vector<CComBSTR> TargetsToSync;
};

struct FSyncThreadWorkerInfo
{
	ColoredTTYEventCallbackPtr OutputCallback;
	vector<wstring> *SrcFilesToSync;
	vector<wstring> *DestFilesToSync;
	CComPtr<IXboxConsole> Target;
	const wchar_t **DirectoriesToCreate;
	int DirectoriesToCreateSize;
};

/**
 * Conversion routinges from string to wstring and back
 */
inline void wstring2string(string& Dest, const wstring& Src)
{
	Dest.assign(Src.begin(), Src.end());
}

inline void string2wstring(wstring& Dest, const string& Src)
{
	Dest.assign(Src.begin(), Src.end());
}

struct FRunGameThreadData
{
	wstring URL;
	wstring MapName;
	wstring Configuration;
	wstring GameName;
	vector<wstring> XenonNames;
	vector<wstring> XenonTargetManagerNames;
};

/**
 * This is the Windows version of a critical section. It uses an aggregate
 * CRITICAL_SECTION to implement its locking.
 */
class FCriticalSection
{
	/**
	 * The windows specific critical section
	 */
	CRITICAL_SECTION CriticalSection;

public:
	/**
	 * Constructor that initializes the aggregated critical section
	 */
	FORCEINLINE FCriticalSection(void)
	{
		InitializeCriticalSection(&CriticalSection);
	}

	/**
	 * Destructor cleaning up the critical section
	 */
	FORCEINLINE ~FCriticalSection(void)
	{
		DeleteCriticalSection(&CriticalSection);
	}

	/**
	 * Locks the critical section
	 */
	FORCEINLINE void Lock(void)
	{
		EnterCriticalSection(&CriticalSection);
	}

	/**
	 * Releases the lock on the critical seciton
	 */
	FORCEINLINE void Unlock(void)
	{
		LeaveCriticalSection(&CriticalSection);
	}
};

/** Logs the specified NULL-terminated buffer to file. */
void DebugOutput(const char* Buffer, const char* Filename)
{
	FILE* DebugFile = NULL;
	OutputDebugString(Buffer);
	const errno_t Result = fopen_s(&DebugFile, Filename, "a");
	if (Result != 0)
	{
		return;
	}
	fprintf(DebugFile, Buffer);
	fclose(DebugFile);
}

/** 
 * Returns a pointer to a subclass of FConsoleSupport.
 *
 * @return The pointer to the console specific FConsoleSupport
 */
CONSOLETOOLS_API FConsoleSupport* GetConsoleSupport()
{
	static FXenonSupport* XenonSupport = NULL;
	if (XenonSupport == NULL)
	{
		XenonSupport = new FXenonSupport();
	}
	if(XenonSupport->IsProperlySetup())
	{
		return XenonSupport;
	}
	else
	{
		return NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////// FXenonSupport /////////////////////////////////////////////////////////////////////////////////////////////

FXenonSupport::FXenonSupport()
{
	XbdmLibrary = NULL;
	bIsProperlySetup = Setup();

	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	MenuItems.push_back(FMenuItem(INVALID_TARGETHANDLE, false)); // reboot xenon
	MenuItems.push_back(FMenuItem(INVALID_TARGETHANDLE, false)); // menu separator
}

FXenonSupport::~FXenonSupport()
{
}

bool FXenonSupport::Setup()
{
	// look for the DLL in the installed path
	char Path[_MAX_PATH];
	DWORD Error = GetEnvironmentVariable("XEDK", Path, sizeof(Path));
	if (Error > 0)
	{
		strcat(Path, "\\bin\\win32\\xbdm.dll");
		XbdmLibrary = LoadLibraryA(Path);
	}
	if( !XbdmLibrary )
	{
		// look for xbdm.dll in the current path
		XbdmLibrary = LoadLibraryA("xbdm.dll");
		if( !XbdmLibrary )
		{
			// if that wasn't found, attempt to find it in the Xenon subdirectory (ie, current directory is Binaries)
			XbdmLibrary = LoadLibraryA("Xenon\\xbdm.dll");
			if( !XbdmLibrary )
			{
				// if still not found, we are done
				OutputDebugString( "Failed to find xbmd.dll. Make sure the XDK is installed or you copy xbdm.dll to Binaries\\Xenon" );
				//MessageBox(NULL, "Failed to find xbmd.dll. Make sure the XDK is installed or you copy xbdm.dll to Binaries\\Xenon", "Can't find xbdm.dll", MB_OK | MB_ICONWARNING);
				return false;
			}
		}
	}

	// Retrieve the version of the xbdm.dll
	INT XDKVersionNumber = 0;
	TCHAR ModuleFilename[MAX_PATH+1];
	if( GetModuleFileName( XbdmLibrary, ModuleFilename, MAX_PATH ) )
	{
		DWORD VersionDataLength = GetFileVersionInfoSize( ModuleFilename, NULL );
		if( VersionDataLength )
		{
			void* VersionData = malloc( VersionDataLength );
			if( GetFileVersionInfo( ModuleFilename, 0, VersionDataLength, VersionData ) )
			{
				LPVOID Value;
				UINT ValueLength;
				if( VerQueryValue( VersionData, TEXT("\\"), &Value, &ValueLength) ) 
				{
					VS_FIXEDFILEINFO* FixedFileInfo = (VS_FIXEDFILEINFO*) Value;
					XDKVersionNumber = HIWORD( FixedFileInfo->dwFileVersionLS );
				}
			}
			free( VersionData );
		}
	}

	// XMAInMemoryEncode doesn't actually encode in memory; it leaves 1000s of temp file droppings that eventually break the system
	// The code here is to workaround that by deleting them on startup
	{
		DWORD Result;
		intptr_t FindResult;
		char TempPath[MAX_PATH] = { 0 };
		char SearchPath[MAX_PATH] = { 0 };
		char FilePath[MAX_PATH] = { 0 };

		// Get temporary path
		Result = GetTempPath( MAX_PATH, TempPath );
		if( Result > 0 && Result < MAX_PATH )
		{
			Result = GetLongPathName( TempPath, TempPath, MAX_PATH );
			if( Result > 0 && Result < MAX_PATH )
			{
				sprintf( SearchPath, "%sEncStrm*", TempPath );

				_finddata32_t FindData;

				FindResult = _findfirst32( SearchPath, &FindData );
				if( FindResult != -1 )
				{
					do 
					{
						sprintf( FilePath, "%s%s", TempPath, FindData.name );
						remove( FilePath );
					} 
					while( !_findnext32( FindResult, &FindData ) );
				}
				_findclose( FindResult );
			}
		}
	}
	return true;
}

void FXenonSupport::Cleanup()
{
	for(set<TARGET>::iterator Iter = CachedConnections.begin(); Iter != CachedConnections.end(); ++Iter)
	{
		VARIANT_BOOL bIsDebugging;
		
		if(SUCCEEDED((*Iter)->get_IsDebugging(&bIsDebugging)) && bIsDebugging)
		{
			(*Iter)->DisconnectDebugger();
		}

		(*Iter)->Release();
	}

	CachedConnections.clear();
	ConnectionNames.clear();
	
	MenuItems.clear();
	MenuItems.push_back(FMenuItem(INVALID_TARGETHANDLE, false)); // reboot xenon
	MenuItems.push_back(FMenuItem(INVALID_TARGETHANDLE, false)); // menu separator

	bIsProperlySetup = false;
}

/**
* Retrieves the target with the specified handle if it exists.
*
* @param	Handle	The handle of the target to retrieve.
*/
TARGET FXenonSupport::GetTarget(TARGETHANDLE Handle)
{
	set<TARGET>::iterator Iter = CachedConnections.find((TARGET)Handle);

	if(Iter != CachedConnections.end())
	{
		return *Iter;
	}

	return NULL;
}

/**
* Retrieves the target with the specified name if it exists.
*
* @param	Handle	The handle of the target to retrieve.
*/
TARGET FXenonSupport::GetTarget(const wchar_t *Name)
{
	for(set<TARGET>::iterator Iter = CachedConnections.begin(); Iter != CachedConnections.end(); ++Iter)
	{
		CComBSTR TempName;
		if(SUCCEEDED((*Iter)->get_Name(&TempName)) && wcscmp(Name, (wchar_t*)TempName.m_str) == 0)
		{
			return *Iter;
		}
	}

	return NULL;
}

/** Initialize the DLL with some information about the game/editor
 *
 * @param	GameName		The name of the current game ("ExampleGame", "UTGame", etc)
 * @param	Configuration	The name of the configuration to run ("Debug", "Release", etc)
 */
void FXenonSupport::Initialize(const wchar_t* InGameName, const wchar_t* InConfiguration)
{
	// cache the parameters
	GameName = InGameName;
	Configuration = InConfiguration;
}

/**
 * Return a string name descriptor for this platform (required to implement)
 *
 * @return	The name of the platform
 */
const wchar_t* FXenonSupport::GetConsoleName()
{
	// this is our hardcoded name
	return CONSOLESUPPORT_NAME_360;
}

/**
 * Return the default IP address to use when sending data into the game for object propagation
 * Note that this is probably different than the IP address used to debug (reboot, run executable, etc)
 * the console. (required to implement)
 *
 * @param	Handle The handle of the console to retrieve the information from.
 *
 * @return	The in-game IP address of the console, in an Intel byte order 32 bit integer
 */
unsigned int FXenonSupport::GetIPAddress(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);
	// make sure it succeeded
	if (!Target)
	{
		return false;
	}

	DWORD TitleIP;
	// ask the console for its game/title ip address
	if (FAILED(Target->get_IPAddressTitle(&TitleIP)))
	{
		// use 0 for error
		TitleIP = 0;
	}

	return TitleIP;
}

/**
 * Return whether or not this console Intel byte order (required to implement)
 *
 * @return	True if the console is Intel byte order
 */
bool FXenonSupport::GetIntelByteOrder()
{
	return false;
}

/**
 * @return the number of known xbox targets
 */
int FXenonSupport::GetNumTargets()
{
	return (int)CachedConnections.size();
}

/**
 * Retrieves a handle to each available target.
 *
 * @param	OutTargetList			An array to copy all of the target handles into.
 * @param	InOutTargetListSize		This variable needs to contain the size of OutTargetList. When the function returns it will contain the number of target handles copied into OutTargetList.
 */
void FXenonSupport::GetTargets(TARGETHANDLE *OutTargetList, int *InOutTargetListSize)
{
	assert(OutTargetList);
	assert(InOutTargetListSize);

	int Index = 0;
	for(set<TARGET>::iterator Iter = CachedConnections.begin(); Iter != CachedConnections.end() && Index < *InOutTargetListSize; ++Iter, ++Index)
	{
		OutTargetList[Index] = (TARGETHANDLE)(*Iter);
	}

	*InOutTargetListSize = Index;
}

/**
 * Get the name of the specified target
 *
 * @param	Handle The handle of the console to retrieve the information from.
 * @return Name of the target, or NULL if the Index is out of bounds
 */
const wchar_t* FXenonSupport::GetTargetName(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		CComBSTR Name;
		
		if(SUCCEEDED(Target->get_Name(&Name)))
		{
			XenonNameCache = (wchar_t*)Name.m_str;
			return XenonNameCache.c_str();
		}
	}

	return NULL;
}

/**
 * @return true if this platform needs to have files copied from PC->target (required to implement)
 */
bool FXenonSupport::PlatformNeedsToCopyFiles()
{
	return true;
}

/**
 * Open an internal connection to a target. This is used so that each operation is "atomic" in 
 * that connection failures can quickly be 'remembered' for one "sync", etc operation, so
 * that a connection is attempted for each file in the sync operation...
 * For the next operation, the connection can try to be opened again.
 *
 * @param TargetName Name of target
 *
 * @return INVALID_TARGETHANDLE for failure, or the handle used in the other functions (MakeDirectory, etc)
 */
TARGETHANDLE FXenonSupport::ConnectToTarget(const wchar_t* TargetName)
{
	TARGET Target = GetTarget(TargetName);

	if(Target)
	{
		CComBSTR DebuggerName(L"XeTools.dll");
		Target->ConnectDebugger(DebuggerName);
	}

	return Target;
}

/**
 * Open an internal connection to a target. This is used so that each operation is "atomic" in 
 * that connection failures can quickly be 'remembered' for one "sync", etc operation, so
 * that a connection is attempted for each file in the sync operation...
 * For the next operation, the connection can try to be opened again.
 *
 * @param Handle The handle of the console to connect to.
 *
 * @return false for failure.
 */
bool FXenonSupport::ConnectToTarget(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);
	bool Result = false;

	if(Target)
	{
		CComBSTR DebuggerName(L"XeTools.dll");
		Result = SUCCEEDED(Target->ConnectDebugger(DebuggerName));
	}

	return Result;
}

/**
* Called after an atomic operation to close any open connections
*
* @param Handle The handle of the console to disconnect.
*/
void FXenonSupport::DisconnectFromTarget(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		Target->DisconnectDebugger();
	}
}

/**
 * Creates a directory
 *
 * @param Handle The handle of the console to retrieve the information from.
 * @param SourceFilename Platform-independent directory name (ie UnrealEngine3\Binaries)
 *
 * @return true if successful
 */
bool FXenonSupport::MakeDirectory(TARGETHANDLE Handle, const wchar_t* DirectoryName)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		wstring FullDir(L"xe:\\");
		FullDir += DirectoryName;

		CComBSTR ComFullDir(FullDir.c_str());

		HRESULT Result = Target->MakeDirectory(ComFullDir);

		bool bRetVal = SUCCEEDED(Result) || (Result == XBDM_ALREADYEXISTS);
		
		return bRetVal;
	}

	return false;
}

/**
 * Determines if the given file needs to be copied
 *
 * @param Handle The handle of the console to retrieve the information from.
 * @param SourceFilename	Path of the source file on PC
 * @param DestFilename		Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
 * @param bReverse			If true, then copying from platform (dest) to PC (src);
 *
 * @return true if successful
 */
bool FXenonSupport::NeedsToCopyFile(TARGETHANDLE Handle, const wchar_t* SourceFilename, const wchar_t* DestFilename, bool bReverse)
{
	TARGET Target = GetTarget(Handle);

	if(!Target)
	{
		return false;
	}

	CComBSTR SourceFilenameBSTR(SourceFilename);
	CComBSTR DestFilenameBSTR(DestFilename);
	VARIANT_BOOL bResult;

	if(FAILED(Target->NeedsToCopyFile(SourceFilenameBSTR, DestFilenameBSTR, bReverse ? VARIANT_TRUE : VARIANT_FALSE, &bResult)))
	{
		return false;
	}

	return bResult == VARIANT_TRUE ? true : false;
}

/**
 * Copies a single file from PC to target
 *
 * @param Handle The handle of the console to retrieve the information from.
 * @param SourceFilename Path of the source file on PC
 * @param DestFilename Platform-independent destination filename (ie, no xe:\\ for Xbox, etc)
 *
 * @return true if successful
 */
bool FXenonSupport::CopyFile(TARGETHANDLE Handle, const wchar_t* SourceFilename, const wchar_t* DestFilename)
{
	TARGET Target = GetTarget(Handle);
	
	// make sure it succeeded
	if (!Target)
	{
		return false;
	}

	// copy the file
	CComBSTR FullDir(L"xe:\\");
	FullDir += DestFilename;

	CComBSTR ComSourceFilename(SourceFilename);

	return SUCCEEDED(Target->SendFile(ComSourceFilename, FullDir));
}

/**
 *	Copies a single file from the target to the PC
 *
 *  @param	Handle			The handle of the console to retrieve the information from.
 *	@param	SourceFilename	Platform-independent source filename (ie, no xe:\\ for Xbox, etc)
 *	@param	DestFilename	Path of the destination file on PC
 *	
 *	@return	bool			true if successful, false otherwise
 */
bool FXenonSupport::RetrieveFile(TARGETHANDLE Handle, const wchar_t* SourceFilename, const wchar_t* DestFilename)
{
	TARGET Target = GetTarget(Handle);

	// make sure it succeeded
	if (!Target)
	{
		return false;
	}

	// copy the file
	CComBSTR FullDir(L"xe:\\");
	FullDir += SourceFilename;

	CComBSTR ComDestFilename(DestFilename);

	return SUCCEEDED(Target->ReceiveFile(ComDestFilename, FullDir));
}

/**
 * Sets the name of the layout file for the DVD, so that GetDVDFileStartSector can be used
 * 
 * @param DVDLayoutFile Name of the layout file
 *
 * @return true if successful
 */
bool FXenonSupport::SetDVDLayoutFile(const wchar_t* DVDLayoutFile)
{
	string LayoutFileString;
	wstring2string(LayoutFileString, DVDLayoutFile);
	// load the file
	return DVDLayout.LoadFile(LayoutFileString.c_str());
}

/**
 * Processes a DVD layout file "object" to see if it's the matching file.
 * 
 * @param Object XmlElement in question
 * @param Filename Filename (no path) we are trying to match
 * @param LBA Output LBA information
 * 
 * @return true if successful
 */
bool FXenonSupport::HandleDVDObject(TiXmlElement* Object, const char* Filename, __int64& LBA)
{
	const char* Name = Object->Attribute("Name");
	if (Name == NULL)
	{
		return false;
	}

	// does the DVD layout filename match what we are looking for?
	if (_stricmp(Filename, Name) == 0)
	{
		// get the LBA attrib
		const char* LBAStr = Object->Attribute("LBA");
		if (LBAStr == NULL)
		{
			return false;
		}

		// cnvert LBA to an int
		LBA = _atoi64(LBAStr);

		return true;
	}

	// if we didn;t get the file, return
	return false;
}

/**
 * Gets the starting sector of a file on the DVD (or whatever optical medium)
 *
 * @param DVDFilename Path to the file on the DVD
 * @param SectorHigh High 32 bits of the sector location
 * @param SectorLow Low 32 bits of the sector location
 * 
 * @return true if the start sector was found for the file
 */
bool FXenonSupport::GetDVDFileStartSector(const wchar_t* DVDFilename, unsigned int& SectorHigh, unsigned int& SectorLow)
{
	// strip off the directory info (we assume that all files are uniquely named, so we ignore the path)
	const wchar_t* FilePart = wcsrchr(DVDFilename, '\\');
	if (FilePart == NULL)
	{
		FilePart = DVDFilename;
	}
	else
	{
		FilePart = FilePart + 1;
	}

	// convert to ascii string
	string DVDFile;
	wstring2string(DVDFile, FilePart);

	// parse the layout for the file of interest
	TiXmlElement* XboxGameDiscLayout = NULL;
	TiXmlElement* Disc = NULL;
	TiXmlElement* Files = NULL;

	// get the layout element
	XboxGameDiscLayout = DVDLayout.FirstChildElement("XboxGameDiscLayout");

	// this element is required
	if (XboxGameDiscLayout == NULL) 
	{
		return false;
	}

	// get the first disc
	Disc = XboxGameDiscLayout->FirstChildElement("Disc");
	// we need at least one
	if (Disc == NULL)
	{
		return false;
	}

	__int64 LBA = -1;
	// loop over all the discs 
	for(; Disc != NULL; Disc = Disc->NextSiblingElement("Disc"))
	{
		// get the first Files element
		Files = Disc->FirstChildElement("Files");

		// we need at least one
		if (Files == NULL)
		{
			return false;
		}

		// loop over all the files groups (one group per layer)
		for (; Files != NULL; Files = Files->NextSiblingElement("Files"))
		{
			// get the first group
			TiXmlElement* Group = Files->FirstChildElement("Group");

			// loop over all groups until we find it
			for(; Group != NULL && LBA == -1; Group = Group->NextSiblingElement("Group")) 
			{
				// get all the files in this group
				TiXmlElement* Object = Group->FirstChildElement("Object");
				for(; Object != NULL; Object = Object->NextSiblingElement("Object")) 
				{
					// see if the object matches, and stop searching if it does
					if (HandleDVDObject(Object, DVDFile.c_str(), LBA))
					{
						break;
					}
				}
			}

			// loop over all the files not in the group until we find it
			TiXmlElement* Object = Files->FirstChildElement("Object");
			for(; Object != NULL && LBA == -1; Object = Object->NextSiblingElement("Object")) 
			{
				// see if the object matches, and stop searching if it does
				if (HandleDVDObject(Object, DVDFile.c_str(), LBA))
				{
					break;
				}
			}
		}
	}

	// if we found the file, return the values
	if (LBA != -1)
	{
		SectorHigh = (unsigned int)(LBA >> 32);
		SectorLow = (unsigned int)(LBA & 0xFFFFFFFF);
		return true;
	}

	return false;
}

/**
 * Reboots the target console. Must be implemented
 *
 * @param Handle The handle of the console to retrieve the information from.
 * 
 * @return true if successful
 */
bool FXenonSupport::Reboot(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);
	// make sure it succeeded
	if (!Target)
	{
		return false;
	}

	// set up empty strings so the reboot doesn't launch anything
	BSTR Empty(NULL);

	// reboot the xbox
	return SUCCEEDED(Target->Reboot(Empty, Empty, Empty, eXboxRebootFlags::Title));
}

/**
 * Reboots the target console. Must be implemented
 *
 * @param Handle			The handle of the console to retrieve the information from.
 * @param Configuration		Build type to run (Debug, Release, RelaseLTCG, etc)
 * @param BaseDirectory		Location of the build on the console (can be empty for platforms that don't copy to the console)
 * @param GameName			Name of the game to run (Example, UT, etc)
 * @param URL				Optional URL to pass to the executable
 * @param bForceGameName	Forces the name of the executable to be only what's in GameName instead of being auto-generated
 * 
 * @return true if successful
 */
bool FXenonSupport::RebootAndRun(TARGETHANDLE Handle, const wchar_t* Configuration, const wchar_t* BaseDirectory, const wchar_t* GameName, const wchar_t* URL, bool bForceGameName)
{
	TARGET Target = GetTarget(Handle);
	// make sure it succeeded
	if (!Target)
	{
		return false;
	}

	// reboot the xbox
	wchar_t ExeName[1024];

	if(bForceGameName)
	{
		swprintf(ExeName, ARRAY_COUNT(ExeName), L"xe:\\%s\\%s", BaseDirectory, GameName);
	}
	else
	{
		swprintf(ExeName, ARRAY_COUNT(ExeName), L"xe:\\%s\\%s-Xe%s.xex", BaseDirectory, GameName, Configuration);
	}

	CComBSTR FullDir(L"xe:\\");
	FullDir += BaseDirectory;

	return SUCCEEDED(Target->Reboot((_bstr_t)ExeName, FullDir, (_bstr_t)URL, eXboxRebootFlags::Title));
}

/**
 * This function is run on a separate thread for cooking, copying, and running an autosaved level on an xbox 360.
 *
 * @param	Data	A pointer to data passed into the thread providing it with extra data needed to do its job.
 */
void __cdecl FXenonSupport::RunGameThread(void *Data)
{
	FRunGameThreadData *ThreadData = (FRunGameThreadData*)Data;

	if(ThreadData && ThreadData->XenonNames.size() > 0 && ThreadData->XenonNames.size() == ThreadData->XenonTargetManagerNames.size())
	{
		wchar_t *XDKDir = _wgetenv(L"XEDK");

		if(XDKDir)
		{
			wstring XBReboot(XDKDir);
			XBReboot += L"\\bin\\win32\\xbreboot.exe";

			/*wstring XBRebootParamXenon(L"/x:");
			XBRebootParamXenon += ThreadData->XenonTargetManagerName;*/

			// NOTE: Shouldn't need this because CookerSync now forces a reboot
			/*ShellExecuteW(NULL, L"open", XBReboot.c_str(), XBRebootParamXenon.c_str(), NULL, SW_HIDE);*/

			wstring AppName;

			if(ThreadData->Configuration == L"Debug")
			{
				AppName = L"debug-";
			}

			AppName += ThreadData->GameName;
			AppName += L".exe";

			wstring CmdLine(L"editor.cookpackages ");
			CmdLine += ThreadData->MapName;
			CmdLine += L" -nopause -platform=xenon -alwaysrecookmaps -alwaysrecookscript";

			PROCESS_INFORMATION ProcInfo;
			STARTUPINFOW StartInfo;

			ZeroMemory(&StartInfo, sizeof(StartInfo));
			StartInfo.cb = sizeof(StartInfo);

			CmdLine = AppName + L" " + CmdLine;

			// Unforunately CreateProcessW() can modify the cmd line string so we have to create a non-const buffer when passing it in
			wchar_t *CmdLineStr = new wchar_t[CmdLine.size() + 1];
			wcscpy_s(CmdLineStr, CmdLine.size() + 1, CmdLine.c_str());

			if(!CreateProcessW(NULL, CmdLineStr, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
			{
				OutputDebugStringW(L"Could not create cooker process!\n");
			}
			else
			{
				WaitForSingleObject(ProcInfo.hProcess, INFINITE);
				CloseHandle(ProcInfo.hThread);
				CloseHandle(ProcInfo.hProcess);

				CmdLine = L"CookerSync.exe ";
				CmdLine += ThreadData->GameName;

				for(size_t i = 0; i < ThreadData->XenonNames.size(); ++i)
				{
					CmdLine += L" ";
					CmdLine += ThreadData->XenonNames[i];
				}

				CmdLine += L" ";
				CmdLine += L"-platform xbox360";

				// Unforunately CreateProcessW() can modify the cmd line string so we have to create a non-const buffer when passing it in
				delete [] CmdLineStr;
				CmdLineStr = new wchar_t[CmdLine.size() + 1];
				wcscpy_s(CmdLineStr, CmdLine.size() + 1, CmdLine.c_str());

				if(!CreateProcessW(NULL, CmdLineStr, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &StartInfo, &ProcInfo))
				{
					OutputDebugStringW(L"Could not create cooker sync process!\n");
				}
				else
				{
					WaitForSingleObject(ProcInfo.hProcess, INFINITE);
					CloseHandle(ProcInfo.hThread);
					CloseHandle(ProcInfo.hProcess);

					wstring XBRebootParms = L" xe:\\UnrealEngine3\\";
					XBRebootParms += ThreadData->GameName;
					XBRebootParms += L"-XeRelease.xex ";
					XBRebootParms += ThreadData->URL;

					for(size_t i = 0; i < ThreadData->XenonTargetManagerNames.size(); ++i)
					{
						CmdLine = L"/x:";
						CmdLine += ThreadData->XenonTargetManagerNames[i];
						CmdLine += XBRebootParms;

						ShellExecuteW(NULL, L"open", XBReboot.c_str(), CmdLine.c_str(), NULL, SW_HIDE);
					}
				}
			}

			delete [] CmdLineStr;
			CmdLineStr = NULL;
		}

		delete ThreadData;
		ThreadData = NULL;
	}
}

/**
 * Run the game on the target console (required to implement)
 *
 * @param	TargetList				The list of handles of consoles to run the game on.
 * @param	NumTargets				The number of handles in TargetList.
 * @param	MapName					The name of the map that is going to be loaded.
 * @param	URL						The map name and options to run the game with
 * @param	OutputConsoleCommand	A buffer that the menu item can fill out with a console command to be run by the Editor on return from this function
 * @param	CommandBufferSize		The size of the buffer pointed to by OutputConsoleCommand
 *
 * @return	Returns true if the run was successful
 */
bool FXenonSupport::RunGame(TARGETHANDLE *TargetList, int NumTargets, const wchar_t* MapName, const wchar_t* URL, wchar_t* /*OutputConsoleCommand*/, int /*CommandBufferSize*/)
{
	FRunGameThreadData *Data = new FRunGameThreadData();

	Data->MapName = MapName;
	Data->GameName = GameName;
	Data->Configuration = Configuration;
	Data->URL = URL;

	for(int i = 0; i < NumTargets; ++i)
	{
		TARGET Target = GetTarget(TargetList[i]);

		if(!Target)
		{
			continue;
		}

		CComBSTR TMName;

		if(FAILED(Target->get_TargetManagerName(&TMName)))
		{
			continue;
		}

		CComBSTR Name;

		if(FAILED(Target->get_Name(&Name)))
		{
			continue;
		}

		Data->XenonNames.push_back((wchar_t*)Name.m_str);
		Data->XenonTargetManagerNames.push_back((wchar_t*)TMName.m_str);
	}

	// Do all cooking, copying, and running on a separate thread so the UI doesn't hang.
	_beginthread(&FXenonSupport::RunGameThread, 0, Data);

	return true;
}

/**
 * Gets a list of targets that have been selected via menu's in UnrealEd.
 *
 * @param	OutTargetList			The list to be filled with target handles.
 * @param	InOutTargetListSize		Contains the size of OutTargetList. When the function returns it contains the # of target handles copied over.
 */
void FXenonSupport::GetMenuSelectedTargets(TARGETHANDLE *OutTargetList, int &InOutTargetListSize)
{
	int CopiedHandles = 0;

	for(size_t i = 0; i < MenuItems.size() && CopiedHandles < InOutTargetListSize; ++i)
	{
		if(MenuItems[i].bChecked)
		{
			OutTargetList[CopiedHandles] = MenuItems[i].Target;
			++CopiedHandles;
		}
	}

	InOutTargetListSize = CopiedHandles;
}

/**
 * Send a text command to the target
 * 
 * @param Handle The handle of the console to retrieve the information from.
 * @param Command Command to send to the target
 */
void FXenonSupport::SendConsoleCommand(TARGETHANDLE Handle, const wchar_t* Command)
{
	TARGET Target = GetTarget(Handle);
	// make sure it succeeded
	if (!Target)
	{
		return;
	}

	// build the text command with channel info
	wstring CommandWithChannel = wstring(L"UNREAL!") + Command;

	CComBSTR Cmd(CommandWithChannel.c_str());

	Target->SendCommand(Cmd);
}

/**
 * Retrieve the state of the console (running, not running, crashed, etc)
 *
 * @param Handle The handle of the console to retrieve the information from.
 *
 * @return the current state
 */
FConsoleSupport::EConsoleState FXenonSupport::GetConsoleState(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);
	_XboxExecutionState CurState;
	FConsoleSupport::EConsoleState RetState = FConsoleSupport::CS_NotRunning;

	if(Target && SUCCEEDED(Target->get_ExecState((LONG*)&CurState)))
	{
		switch(CurState)
		{
		case eXboxExecutionState::Pending:
		case eXboxExecutionState::PendingTitle:
		case eXboxExecutionState::Running:
			{
				RetState = FConsoleSupport::CS_Running;
				break;
			}
		case eXboxExecutionState::Rebooting:
		case eXboxExecutionState::RebootingTitle:
			{
				RetState = FConsoleSupport::CS_Rebooting;
				break;
			}
		case eXboxExecutionState::Stopped:
		default:
			{
				RetState = FConsoleSupport::CS_NotRunning;
				break;
			}
		}
	}

	return RetState;
}

/**
 * Have the console take a screenshot and dump to a file
 * 
 * @param Handle The handle of the console to retrieve the information from.
 * @param Filename Location to place the .bmp file
 *
 * @return true if successful
 */
bool FXenonSupport::ScreenshotBMP(TARGETHANDLE Handle, const wchar_t* Filename)
{
	TARGET Target = GetTarget(Handle);
	// make sure it succeeded
	if (!Target)
	{
		return false;
	}

	// take a screenshot to the specified bmp
	return SUCCEEDED(Target->ScreenShot((_bstr_t)Filename));
}

/**
 * Return the number of console-specific menu items this platform wants to add to the main
 * menu in UnrealEd.
 *
 * @return	The number of menu items for this console
 */
int FXenonSupport::GetNumMenuItems() 
{
	return (int)MenuItems.size();
}

/**
 * Return the string label for the requested menu item
 * @param	Index		The requested menu item
 * @param	bIsChecked	Is this menu item checked (or selected if radio menu item)
 * @param	bIsRadio	Is this menu item part of a radio group?
 *
 * @return	Label for the requested menu item
 */
const wchar_t* FXenonSupport::GetMenuItem(int Index, bool& bIsChecked, bool& bIsRadio, TARGETHANDLE& OutHandle)
{
	// xenon selection is a radio group
	bIsRadio = Index > 1;
	
	if(bIsRadio)
	{
		bIsChecked = MenuItems[Index].bChecked;

		OutHandle = MenuItems[Index].Target;
		TARGET Target = GetTarget(MenuItems[Index].Target);

		if(Target)
		{
			CComBSTR Name;
			if(SUCCEEDED(Target->get_Name(&Name)))
			{
				return (wchar_t*)Name.m_str;
			}
		}
	}
	else
	{
		OutHandle = INVALID_TARGETHANDLE;
	}

	if(Index == 0)
	{
		return L"Reboot Active Xenon";
	}
	else if(Index == 1)
	{
		return MENU_SEPARATOR_LABEL;
	}

	return NULL;
}

/**
 * Internally process the given menu item when it is selected in the editor
 * @param	Index		The selected menu item
 * @param	OutputConsoleCommand	A buffer that the menu item can fill out with a console command to be run by the Editor on return from this function
 * @param	CommandBufferSize		The size of the buffer pointed to by OutputConsoleCommand
 */
void FXenonSupport::ProcessMenuItem(int Index, wchar_t* /*OutputConsoleCommand*/, int /*CommandBufferSize*/)
{
	if(Index == 0)
	{
		BSTR Empty(NULL);

		// this is the first menu item
		for(size_t i = 2; i < MenuItems.size(); ++i)
		{
			if(!MenuItems[i].bChecked)
			{
				continue;
			}

			TARGET Target = GetTarget(MenuItems[i].Target);

			if(Target)
			{
				// reboot the xbox
				Target->Reboot(Empty, Empty, Empty, eXboxRebootFlags::Title);
			}
		}
	}
	else if (Index > 1)
	{
		MenuItems[Index].bChecked = !MenuItems[Index].bChecked;
	}
}

/**
 * Returns the global sound cooker object.
 *
 * @return global sound cooker object, or NULL if none exists
 */
FConsoleSoundCooker* FXenonSupport::GetGlobalSoundCooker() 
{ 
	static FXenonSoundCooker* GlobalSoundCooker = NULL;
	if( !GlobalSoundCooker )
	{
		GlobalSoundCooker = new FXenonSoundCooker();
	}
	return GlobalSoundCooker;
}

/**
 * Returns the global texture cooker object.
 *
 * @return global sound cooker object, or NULL if none exists
 */
FConsoleTextureCooker* FXenonSupport::GetGlobalTextureCooker()
{
	static FXenonTextureCooker* GlobalTextureCooker = NULL;
	if( !GlobalTextureCooker )
	{
		GlobalTextureCooker = new FXenonTextureCooker();
	}
	return GlobalTextureCooker;
}

/**
 * Returns the global skeletal mesh cooker object.
 *
 * @return global skeletal mesh cooker object, or NULL if none exists
 */
FConsoleSkeletalMeshCooker* FXenonSupport::GetGlobalSkeletalMeshCooker() 
{ 
	static FXenonSkeletalMeshCooker* GlobalSkeletalMeshCooker = NULL;
	if( !GlobalSkeletalMeshCooker )
	{
		GlobalSkeletalMeshCooker = new FXenonSkeletalMeshCooker();
	}
	return GlobalSkeletalMeshCooker;
}

/**
 * Returns the global static mesh cooker object.
 *
 * @return global static mesh cooker object, or NULL if none exists
 */
FConsoleStaticMeshCooker* FXenonSupport::GetGlobalStaticMeshCooker() 
{ 
	static FXenonStaticMeshCooker* GlobalStaticMeshCooker = NULL;
	if( !GlobalStaticMeshCooker )
	{
		GlobalStaticMeshCooker = new FXenonStaticMeshCooker();
	}
	return GlobalStaticMeshCooker;
}

/**
 * Returns the global shader precompiler object.
 * @return global shader precompiler object, or NULL if none exists.
 */
FConsoleShaderPrecompiler* FXenonSupport::GetGlobalShaderPrecompiler()
{
	static FXenonShaderPrecompiler* GlobalShaderPrecompiler = NULL;
	if(!GlobalShaderPrecompiler)
	{
		GlobalShaderPrecompiler = new FXenonShaderPrecompiler();
	}
	return GlobalShaderPrecompiler;
}

/**
 * Converts an Unreal Engine texture format to a Xenon texture format.
 *
 * @param	UnrealFormat	The unreal format.
 * @param	Flags			Extra flags describing the format.
 * @return	The associated Xenon format.
 */
D3DFORMAT FXenonSupport::ConvertToXenonFormat(DWORD UnrealFormat, DWORD Flags)
{
	// convert to platform specific format
	D3DFORMAT D3DFormat = ConvertUnrealFormatToD3D(UnrealFormat);
	// sRGB is handled as a surface format on Xe
	if( Flags&TexCreate_SRGB )
	{
		D3DFormat = (D3DFORMAT)MAKESRGBFMT(D3DFormat);
	}	
	// handle un-tiled formats
	if( (Flags&TexCreate_NoTiling) || (Flags & TexCreate_Uncooked) )
	{
		D3DFormat = (D3DFORMAT)MAKELINFMT(D3DFormat);
	}
	return D3DFormat;
}

/**
 *  Gets the platform-specific size required for the given texture.
 *
 *	@param	UnrealFormat	Unreal pixel format
 *	@param	Width			Width of texture (in pixels)
 *	@param	Height			Height of texture (in pixels)
 *	@param	NumMips			Number of miplevels
 *	@param	CreateFlags		Platform-specific creation flags
 *
 *	@return	INT				The size of the memory allocation needed for the texture.
 */
INT FXenonSupport::GetPlatformTextureSize(DWORD UnrealFormat, UINT Width, UINT Height, UINT NumMips, DWORD CreateFlags)
{
	D3DFORMAT Format = ConvertToXenonFormat(UnrealFormat, CreateFlags);
	DWORD D3DCreateFlags = 0;
	if( CreateFlags&TextureCreate_NoPackedMip ) 
	{
		D3DCreateFlags |= XGHEADEREX_NONPACKED;
	}

	D3DTexture DummyTexture;
	// Create a dummy texture we can query the mip alignment from.
	INT TextureSize = XGSetTextureHeaderEx( 
		Width,							// Width
		Height,							// Height
		NumMips,						// Levels
		0,								// Usage
		Format,							// Format
		0,								// ExpBias
		D3DCreateFlags,					// Flags
		0,								// BaseOffset
		XGHEADER_CONTIGUOUS_MIP_OFFSET,	// MipOffset
		0,								// Pitch
		&DummyTexture,					// D3D texture
		NULL,							// unused
		NULL							// unused
		);

	return TextureSize;
}

/**
 *  Gets the platform-specific size required for the given cubemap texture.
 *
 *	@param	UnrealFormat	Unreal pixel format
 *	@param	Size			Size of the cube edge (in pixels)
 *	@param	NumMips			Number of miplevels
 *	@param	CreateFlags		Platform-specific creation flags
 *
 *	@return	INT				The size of the memory allocation needed for the texture.
 */
INT FXenonSupport::GetPlatformCubeTextureSize(DWORD UnrealFormat, UINT Size, UINT NumMips, DWORD CreateFlags)
{
	D3DFORMAT Format = ConvertToXenonFormat(UnrealFormat, CreateFlags);
	DWORD D3DCreateFlags = 0;
	if( CreateFlags&TextureCreate_NoPackedMip ) 
	{
		D3DCreateFlags |= XGHEADEREX_NONPACKED;
	}

	D3DCubeTexture DummyTexture;
	// Create a dummy texture we can query the mip alignment from.
	INT TextureSize = XGSetCubeTextureHeaderEx( 
		Size,							// Size
		NumMips,						// Levels
		0,								// Usage
		Format,							// Format
		0,								// ExpBias
		D3DCreateFlags,					// Flags
		0,								// BaseOffset
		XGHEADER_CONTIGUOUS_MIP_OFFSET,	// MipOffset
		&DummyTexture,					// D3D texture
		NULL,							// unused
		NULL							// unused
		);

	return TextureSize;
}

/**
 * Retrieves the handle of the default console.
 */
TARGETHANDLE FXenonSupport::GetDefaultTarget()
{
	VARIANT_BOOL bIsDefault = VARIANT_FALSE;

	for(set<TARGET>::iterator Iter = CachedConnections.begin(); Iter != CachedConnections.end(); ++Iter)
	{
		if(SUCCEEDED((*Iter)->get_IsDefault(&bIsDefault)) && bIsDefault)
		{
			return *Iter;
		}
	}

	return INVALID_TARGETHANDLE;
}

/**
 * This function exists to delete an instance of FConsoleSupport that has been allocated from a *Tools.dll. Do not call this function from the destructor.
 */
void FXenonSupport::Destroy()
{
	Cleanup();
}

/**
 * Returns true if the specified target is connected for debugging and sending commands.
 * 
 *  @param Handle			The handle of the console to retrieve the information from.
 */
bool FXenonSupport::GetIsTargetConnected(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);
	bool bIsConnected = false;

	if(Target)
	{
		VARIANT_BOOL bCOMIsConnected;

		bIsConnected = SUCCEEDED(Target->get_IsDebugging(&bCOMIsConnected)) && bCOMIsConnected == VARIANT_TRUE;
	}

	return bIsConnected;
}

/**
* Sets the callback function for TTY output.
*
* @param	Callback	Pointer to a callback function or NULL if TTY output is to be disabled.
* @param	Handle		The handle to the target that will register the callback.
*/
void FXenonSupport::SetTTYCallback(TARGETHANDLE Handle, TTYEventCallbackPtr Callback)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		Target->SetTTYCallback((unsigned hyper)Callback);
	}
}

/**
* Sets the callback function for handling crashes.
*
* @param	Callback	Pointer to a callback function or NULL if handling crashes is to be disabled.
* @param	Handle		The handle to the target that will register the callback.
*/
void FXenonSupport::SetCrashCallback(TARGETHANDLE Handle, CrashCallbackPtr Callback)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		Target->SetCrashCallback((unsigned hyper)Callback);
	}
}

/**
 * Retrieves the IP address for the debug channel at the specific target.
 *
 * @param	Handle	The handle to the target to retrieve the IP address for.
 */
unsigned int FXenonSupport::GetDebugChannelIPAddress(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);
	DWORD IPAddr = 0;

	if(Target)
	{
		Target->get_DebugChannelIPAddress(&IPAddr);
	}

	return IPAddr;
}

/**
 * Returns the type of the specified target.
 */
FConsoleSupport::ETargetType FXenonSupport::GetTargetType(TARGETHANDLE Handle)
{
	FConsoleSupport::ETargetType RetVal = FConsoleSupport::TART_Unknown;
	TARGET Target = GetTarget(Handle);
	int TargetType = 0;

	if(Target && SUCCEEDED(Target->get_TargetType(&TargetType)))
	{
		switch(TargetType)
		{
		case eXboxConsoleType::DevelopmentKit:
			{
				RetVal = FConsoleSupport::TART_DevKit;
				break;
			}
		case eXboxConsoleType::TestKit:
			{
				RetVal = FConsoleSupport::TART_TestKit;
				break;
			}
		case eXboxConsoleType::ReviewerKit:
			{
				RetVal = FConsoleSupport::TART_ReviewerKit;
				break;
			}
		}
	}

	return RetVal;
}

/**
 * Sets flags controlling how crash reports will be filtered.
 *
 * @param	Handle	The handle to the target to set the filter for.
 * @param	Filter	Flags controlling how crash reports will be filtered.
 */
bool FXenonSupport::SetCrashReportFilter(TARGETHANDLE Handle, FConsoleSupport::ECrashReportFilter Filter)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		return SUCCEEDED(Target->put_CrashReportFilter(Filter)) ? true : false;
	}

	return false;
}

/**
 * Gets the name of a console as displayed by the target manager.
 *
 * @param	Handle	The handle to the target to set the filter for.
 */
const wchar_t* FXenonSupport::GetTargetManagerNameForConsole(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);

	if(Target)
	{
		CComBSTR TMName;
		if(SUCCEEDED(Target->get_TargetManagerName(&TMName)))
		{
			XenonNameCache = (wchar_t*)TMName.m_str;
			return XenonNameCache.c_str();
		}
	}

	return NULL;
}

/**
 * Enumerates all available targets for the platform.
 *
 * @returns The number of available targets.
 */
int FXenonSupport::EnumerateAvailableTargets()
{
	if ( !bIsProperlySetup )
	{
		OutputDebugString(TEXT("Warning: FXenonSupport::FXenonSupport() failed setup!\n"));
		return (int)CachedConnections.size();
	}

	// create an IXboxManager - the only way to get a list of the xenons in the Xenon Neighborhood!
	CComPtr<IXboxManager> XenonManager;
	HRESULT hr = XenonManager.CoCreateInstance(__uuidof(XboxManager), NULL, CLSCTX_INPROC_SERVER);

	if(FAILED(hr))
	{
		OutputDebugString(TEXT("Warning: FXenonSupport::FXenonSupport() failed to create an instance of XboxManager!\n"));
		return (int)CachedConnections.size();
	}

	// get an IXboxConsoles interface to iterate over all the consoles
	CComPtr<IXboxConsoles> XenonConsoles;
	if(FAILED(XenonManager->get_Consoles(&XenonConsoles)))
	{
		OutputDebugString(TEXT("Warning: FXenonSupport::FXenonSupport() could not retrieve the list of consoles!\n"));
		return (int)CachedConnections.size();
	}

	// get the number of consoles
	LONG NumConsoles = 0;
	if(FAILED(XenonConsoles->get_Count(&NumConsoles)))
	{
		OutputDebugString(TEXT("Warning: FXenonSupport::FXenonSupport() could not retrieve number of consoles. Assuming 0.\n"));
		return (int)CachedConnections.size();
	}
	else if(NumConsoles == 0)
	{
		OutputDebugString(TEXT("Warning: FXenonSupport::FXenonSupport() Number of available consoles is 0, returning.\n"));
		return (int)CachedConnections.size();
	}

	CComBSTR DefaultConsoleName;
	if(FAILED(XenonManager->get_DefaultConsole(&DefaultConsoleName)))
	{
		OutputDebugString(TEXT("Warning: FXenonSupport::FXenonSupport() could not retrieve the default console name. Make sure one is set in xbox neighborhood.\n"));
	}

	// iterate over all of them
	for(LONG ConsoleIndex = 0; ConsoleIndex < NumConsoles; ConsoleIndex++)
	{
		CComBSTR ConsoleName;
		// get the string name of the console
		XenonConsoles->get_Item(ConsoleIndex, &ConsoleName);

		TargetMap::iterator Iter = ConnectionNames.find((wchar_t*)ConsoleName.m_str);

		if(Iter == ConnectionNames.end())
		{
			CComPtr<IXboxConsole> NewTarget;
			if(SUCCEEDED(XenonManager->OpenConsole(ConsoleName, &NewTarget)))
			{
				TARGET NewConsole = NULL;

				NewTarget->put_ConnectTimeout(1000); // default is 5000
				NewTarget->put_ConversationTimeout(1000); // default is 2000

				if(SUCCEEDED(CreateXenonConsole(&NewConsole)))
				{
					VARIANT_BOOL bIsDefault = ConsoleName == DefaultConsoleName ? VARIANT_TRUE : VARIANT_FALSE;
					if(FAILED(NewConsole->Initialize(NewTarget, ConsoleName, bIsDefault)))
					{
						NewConsole->Release();
					}
					else
					{
						ConnectionNames.insert(TargetMap::value_type((wchar_t*)ConsoleName.m_str, NewConsole));
						CachedConnections.insert(NewConsole);

						MenuItems.push_back(FMenuItem((TARGETHANDLE)NewConsole, bIsDefault == VARIANT_TRUE));
					}
				}
			}
		}
	}

	return (int)CachedConnections.size();
}

/**
 * Asynchronously copies a set of files to a set of targets.
 *
 * @param	Handles						An array of targets to receive the files.
 * @param	HandlesSize					The size of the array pointed to by Handles.
 * @param	SrcFilesToSync				An array of source files to copy. This must be the same size as DestFilesToSync.
 * @param	DestFilesToSync				An array of destination files to copy to. This must be the same size as SrcFilesToSync.
 * @param	FilesToSyncSize				The size of the SrcFilesToSync and DestFilesToSync arrays.
 * @param	DirectoriesToCreate			An array of directories to be created on the targets.
 * @param	DirectoriesToCreateSize		The size of the DirectoriesToCreate array.
 * @param	OutputCallback				TTY callback for receiving colored output.
 */
bool FXenonSupport::SyncFiles(TARGETHANDLE *Handles, int HandlesSize, const wchar_t **SrcFilesToSync, const wchar_t **DestFilesToSync, int FilesToSyncSize, const wchar_t **DirectoriesToCreate, int DirectoriesToCreateSize, ColoredTTYEventCallbackPtr OutputCallback)
{
	FSyncThreadInfo *Info = new FSyncThreadInfo;

	Info->DirectoriesToCreate = DirectoriesToCreate;
	Info->DirectoriesToCreateSize = DirectoriesToCreateSize;
	Info->OutputCallback = OutputCallback;

	for(int i = 0; i < HandlesSize; ++i)
	{
		TARGET CurTarget = GetTarget(Handles[i]);
		CComBSTR TMName;

		if(CurTarget && SUCCEEDED(CurTarget->get_TargetManagerName(&TMName)))
		{
			Info->TargetsToSync.push_back(TMName);
		}
	}

	// NOTE: We have to copy file names into another array because this function can return before the copying thread does
	for(int i = 0; i < FilesToSyncSize; ++i)
	{
		Info->SrcFilesToSync.push_back(SrcFilesToSync[i]);
		Info->DestFilesToSync.push_back(DestFilesToSync[i]);
	}

	DWORD ThreadId;
	HANDLE SyncThreadHandle = CreateThread(NULL, 0, SyncThreadMain, Info, 0, &ThreadId);

	if(!SyncThreadHandle)
	{
		// delete info cuz the thread failed which means it won't delete it for us!
		delete Info;
		return false;
	}

	bool bDone = false;
	bool bRetVal = false;

	// The message loop lasts until we get a WM_QUIT message,
	// upon which we shall return from the function.
	while(!bDone)
	{
		// block-local variable 
		DWORD result ; 
		MSG msg ; 

		// Read all of the messages in this next loop, 
		// removing each message as we read it.
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
		{ 
			// If it is a quit message, exit.
			if(msg.message == WM_QUIT)
			{
				CloseHandle(SyncThreadHandle);
				return false;
			}

			// Otherwise, dispatch the message.
			DispatchMessage(&msg); 
		}

		// Wait for any message sent or posted to this queue 
		// or for one of the passed handles be set to signaled.
		result = MsgWaitForMultipleObjects(1, &SyncThreadHandle, FALSE, INFINITE, QS_ALLINPUT); 

		// The result tells us the type of event we have.
		if (result == (WAIT_OBJECT_0 + 1))
		{
			// New messages have arrived. 
			// Continue to the top of the always while loop to 
			// dispatch them and resume waiting.
			continue;
		} 
		else 
		{ 
			DWORD ThreadExitCode;

			bRetVal = GetExitCodeThread(SyncThreadHandle, &ThreadExitCode) && ThreadExitCode == 0;

			// One of the handles became signaled. 
			CloseHandle(SyncThreadHandle);
			bDone = true;
		}
	}

	return bRetVal;
}

/**
 * This handler creates a multi-threaded apartment from which to create all of the target COM objects so we can sync in multiple worker threads.
 *
 * @param	lpParameter		Pointer to extra thread data.
 */
DWORD WINAPI FXenonSupport::SyncThreadMain(LPVOID lpParameter)
{
	FSyncThreadInfo *ThreadData = (FSyncThreadInfo*)lpParameter;

	if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) || ThreadData == NULL)
	{
		delete ThreadData;
		return 1;
	}

	DWORD ReturnCode = 0;
	vector<HANDLE> WorkerThreads;
	vector<CComPtr<IXboxConsole> > Targets;

	CComPtr<IXboxManager> XenonManager;
	if(SUCCEEDED( XenonManager.CoCreateInstance(__uuidof(XboxManager), NULL, CLSCTX_INPROC_SERVER)))
	{
		for(size_t ConsoleIndex = 0; ConsoleIndex < ThreadData->TargetsToSync.size(); ++ConsoleIndex)
		{
			CComPtr<IXboxConsole> Target;
			if(FAILED(XenonManager->OpenConsole(ThreadData->TargetsToSync[ConsoleIndex], &Target)))
			{
				ReturnCode = 1;
			}
			else
			{
				Targets.push_back(Target);
			}
		}
	}

	if(Targets.size() > 0)
	{
		DWORD ThreadId;

		for(size_t i = 0; i < Targets.size(); ++i)
		{
			FSyncThreadWorkerInfo *WorkerInfo = new FSyncThreadWorkerInfo;

			WorkerInfo->SrcFilesToSync = &ThreadData->SrcFilesToSync;
			WorkerInfo->DestFilesToSync = &ThreadData->DestFilesToSync;
			WorkerInfo->Target = Targets[i];
			WorkerInfo->DirectoriesToCreate = ThreadData->DirectoriesToCreate;
			WorkerInfo->DirectoriesToCreateSize = ThreadData->DirectoriesToCreateSize;
			WorkerInfo->OutputCallback = ThreadData->OutputCallback;

			HANDLE WrkrHandle = CreateThread(NULL, 0, SyncThreadWorkerMain, WorkerInfo, 0, &ThreadId);

			if(WrkrHandle)
			{
				WorkerThreads.push_back(WrkrHandle);
			}
			else
			{
				ReturnCode = 1;
				delete WorkerInfo;
			}
		}

		WaitForMultipleObjects((DWORD)WorkerThreads.size(), &WorkerThreads[0], TRUE, INFINITE);

		DWORD ThreadExitCode;

		for(size_t i = 0; i < WorkerThreads.size(); ++i)
		{
			if(GetExitCodeThread(WorkerThreads[i], &ThreadExitCode) && ThreadExitCode != 0)
			{
				ReturnCode = 1;
			}

			CloseHandle(WorkerThreads[i]);
		}

		WorkerThreads.clear();
	}

	Targets.clear();

	CoUninitialize();

	delete ThreadData;

	return ReturnCode;
}

/**
 * Thread for copying PDB's without hanging the UI.
 *
 * @param	lpParameter		Pointer to extra thread data.
 */
DWORD WINAPI FXenonSupport::SyncThreadWorkerMain(LPVOID lpParameter)
{
	FSyncThreadWorkerInfo *ThreadData = (FSyncThreadWorkerInfo*)lpParameter;

	if(FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)) || ThreadData == NULL)
	{
		delete ThreadData;
		return 1;
	}

	CComBSTR ResolvedDestFileToCopy;
	CComBSTR DestFileToCopy;
	CComBSTR SrcFileToCopy;
	CComBSTR TargetName;
	wchar_t OutputBuf[256];
	VARIANT_BOOL bShouldCopy = VARIANT_TRUE;

	ThreadData->Target->get_Name(&TargetName);

	DWORD ReturnCode = 0;

	for(int i = 0; i < ThreadData->DirectoriesToCreateSize; ++i)
	{
		if(FAILED(MakeDirectoryHierarchy(ThreadData->Target, ThreadData->DirectoriesToCreate[i])))
		{
			ReturnCode = 1;
		}
	}

	for(size_t i = 0; i < ThreadData->SrcFilesToSync->size(); ++i)
	{
		DestFileToCopy = (*ThreadData->DestFilesToSync)[i].c_str();
		SrcFileToCopy = (*ThreadData->SrcFilesToSync)[i].c_str();
		ResolvedDestFileToCopy = L"xe:\\";
		ResolvedDestFileToCopy += DestFileToCopy;

		HRESULT hr = S_OK;
		do 
		{
			hr = NeedsToCopyFileInternal(ThreadData->Target, SrcFileToCopy, DestFileToCopy, VARIANT_FALSE, &bShouldCopy);
		}
		while (FAILED(hr) && hr == XBDM_MAXCONNECT);

		if(FAILED(hr) || bShouldCopy == VARIANT_TRUE)
		{
			int Ticks = GetTickCount();

			do 
			{
				hr = ThreadData->Target->SendFile(SrcFileToCopy, ResolvedDestFileToCopy);

				if(SUCCEEDED(hr))
				{
					if(ThreadData->OutputCallback)
					{
						wsprintfW(OutputBuf, L"Copied file \'%s\' to target \'%s\'", (wchar_t*)ResolvedDestFileToCopy.m_str, (wchar_t*)TargetName.m_str);
						//OutputDebugString(OutputBuf);
						ThreadData->OutputCallback(COLOR_BLACK, OutputBuf);
					}
				}
				else if(FAILED(hr) && (hr == XBDM_BADFILENAME || hr == XBDM_NOSUCHFILE || hr == XBDM_NOSUCHPATH))
				{
					ReturnCode = 1;

					if(ThreadData->OutputCallback)
					{
						wsprintfW(OutputBuf, L"Failed to copy file \'%s\' to target \'%s\'. Final error code is 0x%x.", (wchar_t*)ResolvedDestFileToCopy.m_str, (wchar_t*)TargetName.m_str, HRESULT_CODE(hr));
						//OutputDebugString(OutputBuf);
						ThreadData->OutputCallback(COLOR_RED, OutputBuf);
					}

					// end the loop
					hr = S_OK;
				}
			}
			while (FAILED(hr) && GetTickCount() - Ticks < SENDFILE_RETRY);

			if(FAILED(hr) && GetTickCount() - Ticks >= SENDFILE_RETRY)
			{
				ReturnCode = 1;

				if(ThreadData->OutputCallback)
				{
					wsprintfW(OutputBuf, L"Failed to copy file \'%s\' to target \'%s\' due to timeout. Final error code is 0x%x.", (wchar_t*)ResolvedDestFileToCopy.m_str, (wchar_t*)TargetName.m_str, HRESULT_CODE(hr));
					//OutputDebugString(OutputBuf);
					ThreadData->OutputCallback(COLOR_RED, OutputBuf);
				}
			}
		}
#if _DEBUG
		else if(ThreadData->OutputCallback)
		{
			wsprintfW(OutputBuf, L"Skipped file \'%s\' on target \'%s\'", (wchar_t*)ResolvedDestFileToCopy.m_str, (wchar_t*)TargetName.m_str);
			//OutputDebugString(OutputBuf);
			ThreadData->OutputCallback(COLOR_BLACK, OutputBuf);
		}
#endif
	}

	delete ThreadData;

	CoUninitialize();

	return ReturnCode;
}

/**
 * Creates a directory hierarchy on the target.
 *
 * @param	DirectoryHierarchy	The directory hierarchy to be created.
 */
HRESULT FXenonSupport::MakeDirectoryHierarchy(CComPtr<IXboxConsole> &Target, wstring DirectoryHierarchy)
{
	// find the last slash, if there is one
	wstring::size_type LastSlash = DirectoryHierarchy.find_last_of(L'\\');
	HRESULT Result = S_OK;

	// make sure there was a slash, and it wasn't the first character
	if(LastSlash != wstring::npos)
	{
		// if there was a valid slash, make the outer directory
		Result = MakeDirectoryHierarchy(Target, DirectoryHierarchy.substr(0, LastSlash));

		if(FAILED(Result) && Result != XBDM_ALREADYEXISTS)
		{
			return Result;
		}
	}

	// Convert relative path to platform specific one.
	//Folder = BaseDirectory + Folder;

	if(DirectoryHierarchy.find_last_of(L'\\') == DirectoryHierarchy.size() - 1)
	{
		DirectoryHierarchy = DirectoryHierarchy.substr(0, DirectoryHierarchy.size() - 1);
	}

	CComBSTR DirBSTR(L"xe:\\");
	DirBSTR += DirectoryHierarchy.c_str();

	Result = Target->MakeDirectory(DirBSTR);

	if(FAILED(Result) && Result == XBDM_ALREADYEXISTS)
	{
		Result = S_FALSE;
	}

	return Result;
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
HRESULT FXenonSupport::NeedsToCopyFileInternal(CComPtr<IXboxConsole> Target, BSTR SourceFilename, BSTR DestFilename, VARIANT_BOOL bReverse, VARIANT_BOOL *bOutShouldCopy)
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
	HRESULT hr = S_OK;

	if(FAILED(hr = Target->GetFileObject(FullPath, &XeDestinationFile)) && HRESULT_CODE(hr) != XBDM_MAXCONNECT)
	{
		*bOutShouldCopy = VARIANT_TRUE;
		return S_OK;
	}
	else if(FAILED(hr))
	{
		return hr;
	}

	// get xbox file size
	ULONGLONG XboxFileSize;
	if(FAILED(hr = XeDestinationFile->get_Size(&XboxFileSize)))
	{
		return hr;
	}

	// get local file size
	ULONGLONG LocalFileSize = ((ULONGLONG)LocalAttrs.nFileSizeHigh << 32) | ((ULONGLONG)LocalAttrs.nFileSizeLow);
	if (XboxFileSize != LocalFileSize)
	{
		*bOutShouldCopy = VARIANT_TRUE;
		return S_OK;
	}

	// get the xbox file time
	VARIANT Variant;
	if(FAILED(hr = XeDestinationFile->get_ChangeTime(&Variant)))
	{
		return hr;
	}

	// convert it to be useful
	SYSTEMTIME XboxSystemTime;
	VariantTimeToSystemTime(Variant.dblVal, &XboxSystemTime);

	SYSTEMTIME XboxUTCTime;
	TIME_ZONE_INFORMATION TimeZoneInfo;

	GetTimeZoneInformation(&TimeZoneInfo);
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
 * Gets the dump type the current target is set to.
 *
 * @param	Handle	The handle to the target to get the dump type from.
 */
FConsoleSupport::EDumpType FXenonSupport::GetDumpType(TARGETHANDLE Handle)
{
	TARGET Target = GetTarget(Handle);

	if(!Target)
	{
		return FConsoleSupport::DMPT_Normal;
	}

	DWORD DmpFlags;
	FConsoleSupport::EDumpType DmpType = FConsoleSupport::DMPT_Normal;

	if(SUCCEEDED(Target->get_DumpType(&DmpFlags)))
	{
		switch(DmpFlags)
		{
		case eXboxDumpFlags::WithFullMemory:
			{
				DmpType = FConsoleSupport::DMPT_WithFullMemory;
				break;
			}
		}
	}

	return DmpType;
}

/**
 * Sets the dump type of the current target.
 *
 * @param	DmpType		The new dump type of the target.
 * @param	Handle		The handle to the target to set the dump type for.
 */
void FXenonSupport::SetDumpType(TARGETHANDLE Handle, EDumpType DmpType)
{
	TARGET Target = GetTarget(Handle);

	if(!Target)
	{
		return;
	}

	DWORD DmpFlags = eXboxDumpFlags::Normal;

	switch(DmpType)
	{
	case FConsoleSupport::DMPT_WithFullMemory:
		{
			DmpFlags = eXboxDumpFlags::WithFullMemory;
			break;
		}
	}

	Target->put_DumpType(DmpFlags);
}