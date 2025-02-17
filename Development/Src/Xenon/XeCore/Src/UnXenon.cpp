/*=============================================================================
	UnXenon.cpp: Visual C++ Xenon core.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

// Core includes.
#include "CorePrivate.h"
#include "XeD3DDrvPrivate.h"
#include "UnIpDrv.h"
#include "ChartCreation.h"
#include "ProfilingHelpers.h"


#if ALLOW_TRACEDUMP
#pragma comment( lib, "tracerecording.lib" )
#endif

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
#pragma pack(push,8)
#include <xbdm.h>
#pragma pack(pop)
#pragma comment( lib, "xbdm.lib" )
#endif

/*-----------------------------------------------------------------------------
	Vararg.
-----------------------------------------------------------------------------*/

#define VSNPRINTF _vsnwprintf
#define VSNPRINTFA _vsnprintf

/**
* Helper function to write formatted output using an argument list
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgs( TCHAR* Dest, SIZE_T /*DestSize*/, INT Count, const TCHAR*& Fmt, va_list ArgPtr )
{
	INT Result = VSNPRINTF( Dest, Count, Fmt, ArgPtr );
	va_end( ArgPtr );
	return Result;
}

/**
* Helper function to write formatted output using an argument list
* ASCII version
*
* @param Dest - destination string buffer
* @param DestSize - size of destination buffer
* @param Count - number of characters to write (not including null terminating character)
* @param Fmt - string to print
* @param Args - argument list
* @return number of characters written or -1 if truncated
*/
INT appGetVarArgsAnsi( ANSICHAR* Dest, SIZE_T /*DestSize*/, INT Count, const ANSICHAR*& Fmt, va_list ArgPtr )
{
	INT Result = VSNPRINTFA( Dest, Count, Fmt, ArgPtr );
	va_end( ArgPtr );
	return Result;
}

/*-----------------------------------------------------------------------------
	FOutputDeviceWindowsError.
-----------------------------------------------------------------------------*/

//
// Sends the message to the debugging output.
//
void appOutputDebugString( const TCHAR *Message )
{
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	OutputDebugString( Message );
#endif
}

/** Sends a message to a remote tool. */
void appSendNotificationString( const ANSICHAR *Message )
{
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	DmSendNotificationString( Message );
#endif
}

//
// Immediate exit.
//
void appRequestExit( UBOOL Force )
{
	debugf( TEXT("appRequestExit(%i)"), Force );
	if( Force )
	{
		// Don't exit.
		appDebugBreak();
		Sleep( INFINITE );
	}
	else
	{
		// Tell the platform specific code we want to exit cleanly from the main loop.
		GIsRequestingExit = 1;
	}
}

//
// Get system error.
//
const TCHAR* appGetSystemErrorMessage( INT Error )
{
	static TCHAR Msg[1024];
	warnf( TEXT( "GetSystemErrorMessage not supported.  Error: %d" ), Error );
	return Msg;
}

/*-----------------------------------------------------------------------------
	Clipboard.
-----------------------------------------------------------------------------*/

//
// Copy text to clipboard.
//
void appClipboardCopy( const TCHAR* Str )
{
}

//
// Paste text from clipboard.
//
FString appClipboardPaste()
{
	return TEXT("");
}

/*-----------------------------------------------------------------------------
	DLLs.
-----------------------------------------------------------------------------*/

//
// Load a library.
//
void* appGetDllHandle( const TCHAR* Filename )
{
	return NULL;
}

//
// Free a DLL.
//
void appFreeDllHandle( void* DllHandle )
{
}

//
// Lookup the address of a DLL function.
//
void* appGetDllExport( void* DllHandle, const TCHAR* ProcName )
{
	return NULL;
}

void appDebugMessagef( const TCHAR* Fmt, ... )
{
	warnf(TEXT("appDebugMessagef not supported."));
}

/**
 *
 * @param Type - dictates the type of dialog we're displaying
 *
 **/
VARARG_BODY( UBOOL, appMsgf, const TCHAR*, VARARG_EXTRA(EAppMsgType Type) )
{
	TCHAR MsgString[4096]=TEXT("");
	GET_VARARGS( MsgString, ARRAY_COUNT(MsgString), ARRAY_COUNT(MsgString)-1, Fmt, Fmt );

	warnf( TEXT("appMsgf not supported.  Message was: %s" ), MsgString );
	return 1;
}

void appGetLastError( void )
{
	warnf( TEXT("appGetLastError():  Not supported") );
}

// Interface for recording loading errors in the editor
void EdClearLoadErrors()
{
	GEdLoadErrors.Empty();
}

VARARG_BODY( void VARARGS, EdLoadErrorf, const TCHAR*, VARARG_EXTRA(INT Type) )
{
	TCHAR TempStr[4096]=TEXT("");
	GET_VARARGS( TempStr, ARRAY_COUNT(TempStr), ARRAY_COUNT(TempStr)-1, Fmt, Fmt );

	// Check to see if this error already exists ... if so, don't add it.
	// NOTE : for some reason, I can't use AddUniqueItem here or it crashes
	for( INT x = 0 ; x < GEdLoadErrors.Num() ; ++x )
		if( GEdLoadErrors(x) == FEdLoadError( Type, TempStr ) )
			return;

	new( GEdLoadErrors )FEdLoadError( Type, TempStr );
}


/*-----------------------------------------------------------------------------
	Timing.
-----------------------------------------------------------------------------*/

//
// Sleep this thread for Seconds, 0.0 means release the current
// timeslice to let other threads get some attention.
//
void appSleep( FLOAT Seconds )
{
	Sleep( (DWORD)(Seconds * 1000.0) );
}

/**
 * Sleeps forever. This function does not return!
 */
void appSleepInfinite()
{
	Sleep(INFINITE);
}

//
// Return the system time.
//
void appSystemTime( INT& Year, INT& Month, INT& DayOfWeek, INT& Day, INT& Hour, INT& Min, INT& Sec, INT& MSec )
{
	SYSTEMTIME st;
	GetLocalTime( &st );

	Year		= st.wYear;
	Month		= st.wMonth;
	DayOfWeek	= st.wDayOfWeek;
	Day			= st.wDay;
	Hour		= st.wHour;
	Min			= st.wMinute;
	Sec			= st.wSecond;
	MSec		= st.wMilliseconds;
}

/**
* Returns a string with a unique timestamp (useful for creating log filenames)
*/
FString appSystemTimeString( void )
{
	// Create string with system time to create a unique filename.
	INT Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec;

	appSystemTime( Year, Month, DayOfWeek, Day, Hour, Min, Sec, MSec );
	FString	CurrentTime = FString::Printf( TEXT( "%i-%02i-%02i_%02i-%02i-%02i" ), Year, Month, Day, Hour, Min, Sec );

	return( CurrentTime );
}

/*-----------------------------------------------------------------------------
	Profiling.
-----------------------------------------------------------------------------*/

#if ALLOW_TRACEDUMP

/** Filename of current trace. Needs to be kept around as stopping trace will transfer it to host via UnrealConsole. */
FString CurrentTraceFilename;
/** Whether we are currently tracing. */
UBOOL bIsCurrentlyTracing = FALSE;

/**
 * Starts a CPU trace if the passed in trace name matches the global current requested trace.
 *
 * @param	TraceName					trace to potentially start
 * @param	bShouldSuspendGameThread	if TRUE game thread will be suspended for duration of trace
 * @param	bShouldSuspendRenderThread	if TRUE render thread will be suspended for duration of trace
 * @param	TraceSizeInMegs				size of trace buffer size to allocate in MByte
 * @param	FunctionPointer				if != NULL, function pointer if tracing should be limited to single function
 */
void appStartCPUTrace( FName TraceName, UBOOL bShouldSuspendGameThread, UBOOL bShouldSuspendRenderThread, INT TraceSizeInMegs, void* FunctionPointer )
{
	if( TraceName == GCurrentTraceName )
	{
		// Can't start a trace if one is already in progress.
		check( GCurrentTraceName != NAME_None && bIsCurrentlyTracing == FALSE );
		// Keeping track of whether we are tracing independently from trace name as STOP can get called before START.
		GCurrentTraceName = TraceName;
		bIsCurrentlyTracing = TRUE;

		// Use a fixed time stamp for tracing next frame. This allows comparing apples to apples when profiling changes.
		GUseFixedTimeStep = TRUE;

		// Suspend render or game thread if requested. Usually the thread not being profiled is being put to sleep to aide the
		// trace library and not distort results. This is optional though.
		GShouldSuspendGameThread		= bShouldSuspendGameThread;
		GShouldSuspendRenderingThread	= bShouldSuspendRenderThread;

		// Sleep for 500 ms to make sure that the game/ render threads are paused before profiling begins.
		if( GShouldSuspendGameThread || GShouldSuspendRenderingThread )
		{
			appSleep( 0.5 );
		}

		// Set the buffer size to the requested amount. If this OOMs or fails a good trick is to reduce the texture pool size
		// and disable texture streaming if it's orthogonal to the problem being profiled.
		XTraceSetBufferSize( TraceSizeInMegs * 1024 * 1024 );
		// Need to keep the filename around as we're going to copy it to the host later.

		const FString PathName = *( FString(TEXT("GAME:\\")) + FString::Printf( TEXT("%sGame\\Profiling\\"), GGameName )+ TEXT("Trace") PATH_SEPARATOR );
		GFileManager->MakeDirectory( *PathName );

        // needed to correctly create the long set of directories
		const FString CreateDirectoriesString = CreateProfileDirectoryAndFilename( TEXT("Trace"), TEXT(".pix2") );

		const FString Filename = CreateProfileFilename( TEXT(".pix2"), TRUE  );
		const FString FilenameFull = PathName + Filename;

		CurrentTraceFilename = FilenameFull;;

	    CurrentTraceFilename = FString::Printf(TEXT("GAME:\\trace-%s-%s.pix2"), *TraceName.ToString(), *appSystemTimeString());	

		// Start the fun!
		if( FunctionPointer )
		{
			XTraceStartRecordingFunction( TCHAR_TO_ANSI(*CurrentTraceFilename ), FunctionPointer, TRUE );
		}
		else
		{
			XTraceStartRecording( TCHAR_TO_ANSI(*CurrentTraceFilename ) );
		}
	}
}

/**
 * Stops tracing if currently active and matching trace name.
 *
 * @param	TraceToStop		name of trace to stop
 */
void appStopCPUTrace( FName TraceToStop )
{
	// It's okay to stop tracing even if no trace is running as it makes the code flow easier.
	if( bIsCurrentlyTracing && TraceToStop == GCurrentTraceName )
	{
		check( GCurrentTraceName != NAME_None );

		// We're no longer performing a trace
		bIsCurrentlyTracing = FALSE;

		// Stop the trace.
		GCurrentTraceName = NAME_None;
		XTraceStopRecording();

		// Notify Unreal console of newly available trace to copy to PC.
		SendDataToPCViaUnrealConsole( TEXT("UE_PROFILER!GAME:"), CurrentTraceFilename );

		// Unsuspend render and game threads now that we're done.
		GShouldSuspendGameThread		= FALSE;
		GShouldSuspendRenderingThread	= FALSE;
	}
}

#endif


/*-----------------------------------------------------------------------------
	Link functions.
-----------------------------------------------------------------------------*/

//
// Launch a uniform resource locator (i.e. http://www.epicgames.com/unreal).
// This is expected to return immediately as the URL is launched by another
// task.
//
void appLaunchURL( const TCHAR* URL, const TCHAR* Parms, FString* Error )
{
	warnf( TEXT( "appLaunchURL() was called" ) );
}

void* appCreateProc( const TCHAR* URL, const TCHAR* Parms )
{
	appErrorf(TEXT("appCreateProc not supported."));
	return NULL;
}

UBOOL appGetProcReturnCode( void* ProcHandle, INT* ReturnCode )
{
	appErrorf(TEXT("appGetProcReturnCode not supported."));
	return false;
}

/** Returns TRUE if the specified application is running */
UBOOL appIsApplicationRunning( DWORD ProcessId )
{
	appErrorf(TEXT("appIsApplicationRunning not implemented."));
	return FALSE;
}

/*-----------------------------------------------------------------------------
	File handling.
-----------------------------------------------------------------------------*/

void appCleanFileCache()
{
	// do standard cache cleanup
	GSys->PerformPeriodicCacheCleanup();

	// perform any other platform-specific cleanup here
}

/**
 * Handles IO failure by either signaling an event for the render thread or handling
 * the event from the render thread.
 *
 * @param Filename	If not NULL, name of the file the I/O error occured with
 */
void appHandleIOFailure( const TCHAR* Filename )
{
	appErrorf(TEXT("I/O failure operating on '%s'"), Filename ? Filename : TEXT("Unknown file"));

	// Signals to the rendering thread to handle the dirty disc error and 
	// signals to other threads to stop doing anything meaningful
	GHandleDirtyDiscError = TRUE;

	if (IsInRenderingThread())
	{
		RHISuspendRendering();
		XShowDirtyDiscErrorUI(0); //@todo TCR: use the proper index
	}
	else
	{
		// Stall indefinitely on this thread
		appSleepInfinite();
	}
}

/*-----------------------------------------------------------------------------
	Guids.
-----------------------------------------------------------------------------*/

//
// Create a new globally unique identifier.
//
FGuid appCreateGuid()
{
	//@todo xenon: appCreateGuid()
	SYSTEMTIME Time;
	GetLocalTime( &Time );

	FGuid	GUID;
	GUID.A = Time.wDay | (Time.wHour << 16);
	GUID.B = Time.wMonth | (Time.wSecond << 16);
	GUID.C = Time.wMilliseconds | (Time.wMinute << 16);
	GUID.D = Time.wYear ^ appCycles();
	return GUID;
}

/*-----------------------------------------------------------------------------
	Command line.
-----------------------------------------------------------------------------*/

// Get startup directory.
const TCHAR* appBaseDir()
{
	static TCHAR Result[256]=TEXT("");
	return Result;
}

// Get computer name.
const TCHAR* appComputerName()
{
	static TCHAR Result[256]=TEXT("Xenon");
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	char Temp[256];
	DWORD Num = 256;
	// Get the real xbox name
	if (DmGetXboxName(Temp,&Num) == XBDM_NOERR)
	{
		// Copy it into the static
		appStrcpy(Result,ANSI_TO_TCHAR(Temp));
	}
#endif
	return Result;
}

// Get user name.
const TCHAR* appUserName()
{
	static TCHAR Result[256]=TEXT("User");
	return Result;
}

// shader dir relative to appBaseDir
const TCHAR* appShaderDir()
{
	static TCHAR Result[256] = PATH_SEPARATOR TEXT("Engine") PATH_SEPARATOR TEXT("Shaders");
	return Result;
}

/** Returns name of currently running executable. */
const TCHAR* appExecutableName()
{
	static TCHAR Result[256]=TEXT("Unknown");
#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	DM_XBE XBEInfo;
	if( SUCCEEDED(DmGetXbeInfo("",&XBEInfo)) )
	{
		FFilename ExecutableName = XBEInfo.LaunchPath;
		appStrncpy(Result,*ExecutableName.GetBaseFilename(),ARRAY_COUNT(Result)) ;
	}
#endif
	return Result;
}


/*-----------------------------------------------------------------------------
	SHA-1 functions.
-----------------------------------------------------------------------------*/

/** Global map of filename to hash value */
TMap<FString, BYTE*> GSHAHashMap;

/**
 * Get the hash values out of the executable hash section
 *
 * NOTE: To put hash keys into the executable, you will need to put a line like the
 *		 following into your xbox linker settings:
 *			/XEXSECTION:HashSec=..\..\..\MyGame\CookedXenon\Hashes.sha
 *
 *		 Then, use the -sha option to the cooker (must be from commandline, not
 *       frontend) to generate the hashes for .ini, loc, and startup packages
 *
 *		 You probably will want to make and checkin an empty file called Hashses.sha
 *		 into your source control to avoid linker warnings. Then for testing or final
 *		 final build ONLY, use the -sha command and relink your executable to put the
 *		 hashes for the current files into the executable.
 */
void InitSHAHashes()
{
	void* SectionData;
	DWORD SectionSize;
	// look for the hash section
	if (XGetModuleSection(GetModuleHandle(NULL), "HashSec", &SectionData, &SectionSize))
	{
		// there may be a dummy byte for platforms that can't handle empty files for linking
		if (SectionSize <= 1)
		{
			return;
		}
		// if it exists, parse it
		DWORD Offset = 0;
		while (Offset < SectionSize)
		{
			// format is null terminated string followed by hash
			ANSICHAR* Filename = (ANSICHAR*)SectionData + Offset;
			
			// make sure it's not an empty string (this could happen with an empty hash file)
			if (Filename[0])
			{
				// skip over the file
				Offset += strlen(Filename) + 1;

				// offset now points to the hash data, so save a pointer to it
				GSHAHashMap.Set(ANSI_TO_TCHAR(Filename), (BYTE*)SectionData + Offset);

				// move the offset over the hash (always 20 bytes)
				Offset += 20;
			}
		}
		// we should be exactly at the end
		check(Offset == SectionSize);
	}
}

/**
 * Gets the stored SHA hash from the platform, if it exists. This function
 * must be able to be called from any thread.
 *
 * @param Pathname Pathname to the file to get the SHA for
 * @param Hash 20 byte array that receives the hash
 *
 * @return TRUE if the hash was found, FALSE otherwise
 */
UBOOL appGetFileSHAHash(const TCHAR* Pathname, BYTE Hash[20])
{
	// look for this file in the hash
	BYTE** HashData = GSHAHashMap.Find(FFilename(Pathname).GetCleanFilename().ToLower());
	// did we find it?
	if (HashData)
	{
		// return it
		appMemcpy(Hash, *HashData, 20);
	}

	// return TRUE if we found the hash
	return HashData != NULL;
}

/**
 * Callback that is called if the asynchronous SHA verification fails
 * This will be called from a pooled thread.
 *
 * @param FailedPathname Pathname of file that failed to verify
 * @param bFailedDueToMissingHash TRUE if the reason for the failure was that the hash was missing, and that was set as being an error condition
 */
void appOnFailSHAVerification(const TCHAR* FailedPathname, UBOOL bFailedDueToMissingHash)
{
 	debugf(TEXT("SHA Verification failed for '%s'. Reason: %s"), 
 		FailedPathname ? FailedPathname : TEXT("Unknown file"),
 		bFailedDueToMissingHash ? TEXT("Missing hash") : TEXT("Bad hash"));

	// popup the dirty disc message
	XShowDirtyDiscErrorUI(0);
}

/*-----------------------------------------------------------------------------
	App init/exit.
-----------------------------------------------------------------------------*/

/** Whether networking has been successfully initialized. Not clobbered by the memory image code.				*/
UBOOL					GIpDrvInitialized;

#include "UnThreadingWindows.h"
static FSynchronizeFactoryWin	SynchronizeFactory;
static FThreadFactoryWin		ThreadFactory;
/** Thread pool for shared async work */
static FQueuedThreadPoolWin		ThreadPool;

QWORD GCyclesPerSecond = 1;

/** Does Xenon specific initialization of timing information */
void appInitTiming(void)
{
	LARGE_INTEGER Frequency;
	verify( QueryPerformanceFrequency(&Frequency) );
	GCyclesPerSecond = Frequency.QuadPart;
	GSecondsPerCycle = 1.0 / Frequency.QuadPart;
	GStartTime = appSeconds();
}

/**
 * Xenon specific initialization we do up- front.
 */
void appXenonInit( const TCHAR* CommandLine )
{
	// Set file meta data cache to 128 KByte (default is 64 KByte).
	XSetFileCacheSize( 128 * 1024 );

	// copy the command line since we need it before appInit gets called
	extern TCHAR GCmdLine[16384];
	appStrncpy( GCmdLine, CommandLine, 16384 );

	//
	//	Initialiaze threading.
	// 

	// Initialiaze global threading and synchronization factories. This is done in FEngineLoop::Init on PC.
	GSynchronizeFactory			= &SynchronizeFactory;
	GThreadFactory				= &ThreadFactory;
	GThreadPool					= &ThreadPool;
	// Create the pool of threads that will be used (see UnXenon.h)
	verify(GThreadPool->Create(THREAD_POOL_COUNT,THREAD_POOL_HWTHREAD));

	//
	//	Initialize D3D.
	//
#if USE_XeD3D_RHI
	XeInitD3DDevice();	
#endif // USE_XeD3D_RHI
	//
	//	Initialize XAudio.
	//

	XAUDIOENGINEINIT AudioInit					= { 0 };
	AudioInit.SubmixStageCount					= 2;
	AudioInit.pEffectTable						= &XAudioDefaultEffectTable;
	AudioInit.MaxVoiceChannelCount				= 9;
    AudioInit.SubmixStageCount					= 3;
	AudioInit.ThreadUsage						= AUDIO_HWTHREAD;
	verify( SUCCEEDED( XAudioInitialize( &AudioInit ) ) );
	XAudioSetDebugBreakLevel( XAUDIODEBUGLEVEL_NONE );

	// initialize stuff for the splash screen
	appXenonInitSplash();

	// pull SHA hashes from the executable
	InitSHAHashes();
}

void appPlatformPreInit()
{
}

void appPlatformInit()
{
	// Randomize.
	srand( 0 );
   
	// Identity.
	debugf( NAME_Init, TEXT("Computer: %s"), appComputerName() );
	debugf( NAME_Init, TEXT("User: %s"), appUserName() );

	// Timer resolution.
	debugf( NAME_Init, TEXT("High frequency timer resolution =%f MHz"), 0.000001 / GSecondsPerCycle );
 
	// Get memory.
	MEMORYSTATUS M;
	GlobalMemoryStatus(&M);
	debugf( NAME_Init, TEXT("Memory total: Phys=%iK Pagef=%iK Virt=%iK"), M.dwTotalPhys/1024, M.dwTotalPageFile/1024, M.dwTotalVirtual/1024 );

	// Get CPU info.
	GNumHardwareThreads = 6;

	// Load the DrawUP check counts from the configuration file
#if !FINAL_RELEASE
	if (GConfig)
	{
		INT Temporary = 0;
		if (GConfig->GetInt(TEXT("XeD3D"), TEXT("DrawUPVertexCheckCount"), Temporary, GEngineIni) == TRUE)
		{
			GDrawUPVertexCheckCount = Temporary;
		}
		if (GConfig->GetInt(TEXT("XeD3D"), TEXT("DrawUPIndexCheckCount"), Temporary, GEngineIni) == TRUE)
		{
			GDrawUPIndexCheckCount = Temporary;
		}
	}
#endif

#if USE_XeD3D_RHI
	// Check for, and set if present, RingBuffer configuration
	XeSetRingBufferParametersFromConfig();
	
	if (GConfig)
	{
		// Texture memory sharing setting
		GConfig->GetBool(TEXT("XeD3D"), TEXT("bShareTextureMipMemory"), GTexturesCanShareMipMemory, GEngineIni);
		// Corrective gamma ramp seetting
		GConfig->GetBool(TEXT("XeD3D"), TEXT("bUseCorrectiveGammaRamp"), GUseCorrectiveGammaRamp, GEngineIni);
		XeSetCorrectiveGammaRamp(GUseCorrectiveGammaRamp);
	}
#endif  // USE_XeD3D_RHI
}

void appPlatformPostInit()
{
#if STATS
	// Read the total physical memory available, and tell the stat manager
	MEMORYSTATUS MemStatus;
	GlobalMemoryStatus(&MemStatus);
	GStatManager.SetAvailableMemory(MCR_Physical, MemStatus.dwTotalPhys);
#endif
}


#include "zlib.h"

/**
 * zlib's memory allocator, overridden to use our allocator.
 *
 * @param	Opaque		unused
 * @param	NumItems	number of items to allocate
 * @param	Size		size in bytes of a single item
 *
 * @return	pointer to allocated memory
 */
extern "C" voidpf zcalloc( voidpf* /*Opaque*/, unsigned int NumItems, unsigned int Size )
{
	return appMalloc( NumItems * Size );
}

/**
 * zlib's memory de-allocator, overridden to use our allocator.
 *
 * @param	Opaque		unused
 * @param	Pointer		pointer to block of memory to free
 */
extern "C" void zcfree( voidpf /*Opaque*/, voidpf Pointer )
{
	appFree( Pointer );
}

/**
 * @return		The language extension to use for Portuguese, as specified in the engine ini, or POR if non was specified.
 */
static const TCHAR* GetPortugueseLanguageExt()
{
	static FString PortugueseLanguageExt(TEXT("POR"));
	GConfig->GetString( TEXT("Engine.Engine"), TEXT("PortugueseLanguageExt"), PortugueseLanguageExt, GEngineIni );
	const TCHAR* RetVal = *PortugueseLanguageExt;
	return RetVal;
}

/** 
 * Returns the language setting that is configured for the platform 
 *
 * See GKnownLanguageExtensions for the definitive list
 */
FString appGetLanguageExt( void )
{
	static FString LangExt = TEXT( "" );

	if( LangExt.Len() )
	{
		return( LangExt );
	}

	// Get the language registered in the dash.
	const DWORD LangIndex = XGetLanguage();
	const DWORD LocaleIndex = XGetLocale();

	// The dashboard has only one Spanish language setting.  So, check the region to
	// determine whether we should use Iberian (Europe) or Latam (North America).

	FString LanguageExtension = TEXT( "INT" );
	switch( LangIndex )
	{
	case XC_LANGUAGE_ENGLISH:
		LanguageExtension = TEXT( "INT" );
		break;
	// case XC_LANGUAGE_JAPANESE:
	//	LanguageExtension = TEXT( "JPN" );
	//	break;
	case XC_LANGUAGE_GERMAN:
		LanguageExtension = TEXT( "DEU" );
		break;
	case XC_LANGUAGE_FRENCH:
		LanguageExtension = TEXT( "FRA" );
		break;
	case XC_LANGUAGE_SPANISH:
		if( LocaleIndex == XC_LOCALE_SPAIN )
		{
			LanguageExtension = TEXT( "ESN" );
		}
		else
		{
			LanguageExtension = TEXT( "ESM" );
		}
		break;
	case XC_LANGUAGE_ITALIAN:
		LanguageExtension = TEXT( "ITA" );
		break;
	case XC_LANGUAGE_KOREAN:
		LanguageExtension = TEXT( "KOR" );
		break;
	case XC_LANGUAGE_SCHINESE:
	case XC_LANGUAGE_TCHINESE:
		LanguageExtension = TEXT( "CHT" );
		break;
	case XC_LANGUAGE_POLISH:
		LanguageExtension = TEXT( "POL" );
		break;
	// case XC_LANGUAGE_PORTUGUESE:
	//	// So this can be set in the ini
	//	LanguageExtension = GetPortugueseLanguageExt();
	//	break;
	case XC_LANGUAGE_RUSSIAN:
		LanguageExtension = TEXT( "RUS" );
		break;
	}

	// Always override with the CZE, SLO or HUN locales
	switch( LocaleIndex )
	{
	case XC_LOCALE_CZECH_REPUBLIC:
	case XC_LOCALE_SLOVAK_REPUBLIC:
		LanguageExtension = TEXT( "CZE" );
		break;
	case XC_LOCALE_HUNGARY:
		LanguageExtension = TEXT( "HUN" );
		break;
	}

	// Special case handling of Spanish
	if( LanguageExtension == TEXT( "ESN" ) || LanguageExtension == TEXT( "ESM" ) )
	{
		// If the full loc version of the requested Spanish does not exist, try the other type
		FString TOCFileName = FString::Printf( TEXT( "GearGame\\Xbox360TOC_%s.txt" ), *LanguageExtension );
		if( GFileManager->FileSize( *TOCFileName ) == -1 )
		{
			if( LanguageExtension == TEXT( "ESN" ) )
			{
				LanguageExtension = TEXT( "ESM" );
			}
			else
			{
				LanguageExtension = TEXT( "ESN" );
			}
		}
	}

	LangExt = LanguageExtension;
	return( LangExt );
}

/*-----------------------------------------------------------------------------
	Misc
-----------------------------------------------------------------------------*/
#include "UMemoryDefines.h"

/**
* Converts the passed in program counter address to a human readable string and appends it to the passed in one.
* @warning: The code assumes that HumanReadableString is large enough to contain the information.
*
* @param	ProgramCounter			Address to look symbol information up for
* @param	HumanReadableString		String to concatenate information with
* @param	HumanReadableStringSize size of string in characters
* @param	VerbosityFlags			Bit field of requested data for output. -1 for all output.
*/ 
void appProgramCounterToHumanReadableString( QWORD ProgramCounter, ANSICHAR* HumanReadableString, SIZE_T HumanReadableStringSize, EVerbosityFlags VerbosityFlags )
{
    if (HumanReadableString && HumanReadableStringSize > 0)
    {
        ANSICHAR TempArray[MAX_SPRINTF];
        appSprintfANSI(TempArray, "[Xbox360Callstack] 0x%.8x\n", (DWORD) ProgramCounter);
        appStrcatANSI( HumanReadableString, HumanReadableStringSize, TempArray );

        if( VerbosityFlags & VF_DISPLAY_FILENAME )
        {
            //Append the filename to the string here
        }
    }
}

/**
 * Capture a stack backtrace and optionally use the passed in exception pointers.
 *
 * @param	BackTrace			[out] Pointer to array to take backtrace
 * @param	MaxDepth			Entries in BackTrace array
 * @param	Context				Optional thread context information
 * @return	Number of function pointers captured
 */
DWORD appCaptureStackBackTrace( QWORD* BackTrace, DWORD MaxDepth, void* Context)
{
	// Zero out all entries.
	appMemzero( BackTrace, sizeof(QWORD) * MaxDepth );

#if ALLOW_NON_APPROVED_FOR_SHIPPING_LIB
	check(sizeof(PVOID*)<=sizeof(QWORD));
	PVOID* DmBackTrace = (PVOID*) BackTrace;

	// capture the stack
	if (DmCaptureStackBackTrace(MaxDepth, DmBackTrace) ==  XBDM_NOERR)
	{
		// Expand from PVOID* to QWORD in reverse order.
		for( INT i=MaxDepth-1; i>=0; i-- )
		{
			BackTrace[i] = (QWORD) DmBackTrace[i];
		}

		// count depth
		DWORD Depth;
		for (Depth = 0; Depth < MaxDepth; Depth++)
		{
			// go until we hit a zero
			if (BackTrace[Depth] == 0)
			{
				break;
			}
		}
		return Depth;
	}
	else
	{
		return 0;
	}
#else
	return 0;
#endif
}


/** 
 * This will update the passed in FMemoryChartEntry with the platform specific data
 *
 * @param FMemoryChartEntry the struct to fill in
 **/
void appUpdateMemoryChartStats( struct FMemoryChartEntry& MemoryEntry )
{
	//@todo fill in these values

	// set the memory chart data
	MemoryEntry.GPUMemUsed = 0;

	MemoryEntry.NumAllocations = 0;
	MemoryEntry.AllocationOverhead = 0;
	MemoryEntry.AllignmentWaste = 0;
}






