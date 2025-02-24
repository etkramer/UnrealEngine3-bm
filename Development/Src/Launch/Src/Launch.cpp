/*=============================================================================
	Launch.cpp: Game launcher.
	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
=============================================================================*/

#include "LaunchPrivate.h"

#if _WINDOWS

#define HAVE_WXWIDGETS 1

#if HAVE_WXWIDGETS
// use wxWidgets as a DLL
#include <wx/evtloop.h>  // has our base callback class
#endif

FEngineLoop	GEngineLoop;
/** Whether to use wxWindows when running the game */
UBOOL		GUsewxWindows;

/** Whether we should pause before exiting. used by UCC */
UBOOL		GShouldPauseBeforeExit;

#if STATS
/** Global for tracking FPS */
FFPSCounter GFPSCounter;
#endif 
 
// !!! FIXME: remove this.
#if PLATFORM_UNIX
#define appShowSplash(x) STUBBED("appShowSplash")
#define appHideSplash() STUBBED("appHideSplash")
#endif

extern "C" int test_main(int argc, char ** argp)
{
	return 0;
}

/*-----------------------------------------------------------------------------
	WinMain.
-----------------------------------------------------------------------------*/

#if _WINDOWS
extern TCHAR MiniDumpFilenameW[1024];
extern char  MiniDumpFilenameA[1024];
extern INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo );

// use wxWidgets as a DLL
extern bool IsUnrealWindowHandle( HWND hWnd );
#endif

#if HAVE_WXWIDGETS
class WxUnrealCallbacks : public wxEventLoopBase::wxUnrealCallbacks
{
public:

	virtual bool IsUnrealWindowHandle(HWND hwnd) const
	{
		return ::IsUnrealWindowHandle(hwnd);
	}

	virtual bool IsRequestingExit() const 
	{ 
		return GIsRequestingExit ? true : false; 
	}

	virtual void SetRequestingExit( bool bRequestingExit ) 
	{ 
		GIsRequestingExit = bRequestingExit ? true : false; 
	}

};

static WxUnrealCallbacks s_UnrealCallbacks;
#endif


/**
 * Performs any required cleanup in the case of a fatal error.
 */
static void StaticShutdownAfterError()
{
	// Make sure Novodex is correctly torn down.
	DestroyGameRBPhys();

#if WITH_FACEFX
	// Make sure FaceFX is shutdown.
	UnShutdownFaceFX();
#endif // WITH_FACEFX

#if HAVE_WXWIDGETS
	// Unbind DLLs (e.g. SCC integration)
	WxLaunchApp* LaunchApp = (WxLaunchApp*) wxTheApp;
	if( LaunchApp )
	{
		LaunchApp->ShutdownAfterError();
	}
#endif
}

/**
* Sets global to TRUE if the app should pause infinitely before exit.
* Currently used by UCC.
*/
void SetShouldPauseBeforeExit(INT ErrorLevel)
{
	// If we are UCC, determine 
	if( GIsUCC )
	{
		// UCC.
		UBOOL bInheritConsole = FALSE;

#if !CONSOLE
		if(NULL != GLogConsole)
		{
			// if we're running from a console we inherited, do not sleep indefinitely
			bInheritConsole = GLogConsole->IsInherited();
		}
#endif

		// Either close log window manually or press CTRL-C to exit if not in "silent" or "nopause" mode.
		GShouldPauseBeforeExit = !bInheritConsole && !GIsSilent && !ParseParam(appCmdLine(),TEXT("NOPAUSE"));
		// if it was specified to not pause if successful, then check that here
#if BATMAN
		// We don't need to leave the app running to debug it.
#else
		if (ParseParam(appCmdLine(),TEXT("NOPAUSEONSUCCESS")) && ErrorLevel == 0)
#endif
		{
			// we succeeded, so don't pause 
			GShouldPauseBeforeExit = FALSE;
		}
	}
}

/**
 * Static guarded main function. Rolled into own function so we can have error handling for debug/ release builds depending
 * on whether a debugger is attached or not.
 */
static INT GuardedMain( const TCHAR* CmdLine, HINSTANCE hInInstance, HINSTANCE hPrevInstance, INT nCmdShow )
{
	// make sure GEngineLoop::Exit() is always called.
	struct EngineLoopCleanupGuard  { ~EngineLoopCleanupGuard() { GEngineLoop.Exit(); } } CleanupGuard;

	// Set up minidump filename. We cannot do this directly inside main as we use an FString that requires 
	// destruction and main uses SEH.
	// These names will be updated as soon as the Filemanager is set up so we can write to the log file.
	// That will also use the user folder for installed builds so we don't write into program files or whatever.
#if _WINDOWS
	appStrcpy( MiniDumpFilenameW, *FString::Printf( TEXT("unreal-v%i-%s.dmp"), GEngineVersion, *appSystemTimeString() ) );
	appStrcpyANSI( MiniDumpFilenameA, TCHAR_TO_ANSI( MiniDumpFilenameW ) );

	CmdLine = RemoveExeName(CmdLine);
#endif


	INT ErrorLevel	= GEngineLoop.PreInit( CmdLine );
	
	SetShouldPauseBeforeExit(ErrorLevel);

	GUsewxWindows = 0;
#if HAVE_WXWIDGETS
	GUsewxWindows	= GIsEditor || ParseParam(appCmdLine(),TEXT("WXWINDOWS")) || ParseParam(appCmdLine(),TEXT("REMOTECONTROL"));
#endif

	// exit if PreInit failed.
	if ( ErrorLevel != 0 || GIsRequestingExit )
	{
		return ErrorLevel;
	}

	if( GUsewxWindows )
	{
#if HAVE_WXWIDGETS
		// use wxWidgets as a DLL
		// set the call back class here
		wxEventLoopBase::SetUnrealCallbacks( &s_UnrealCallbacks );

		// UnrealEd of game with wxWindows.
		ErrorLevel = wxEntry( hInInstance, hPrevInstance, "", nCmdShow);
#endif
	}
	else
	{
		// Game without wxWindows.
		if( !ParseParam(appCmdLine(),TEXT("NOSPLASH")) )
		{
			FString SplashPath;
			if (appGetSplashPath(GIsEditor ? TEXT("PC\\EdSplash.bmp") : TEXT("PC\\Splash.bmp"), SplashPath) == TRUE)
			{
				appShowSplash( *SplashPath );
			}
		}
		ErrorLevel = GEngineLoop.Init();
		if ( !GIsGame )
		{
			appHideSplash();
		}

		while( !GIsRequestingExit )
		{
#if STATS
			GFPSCounter.Update(appSeconds());
#endif
			{
				SCOPE_CYCLE_COUNTER(STAT_FrameTime);
				GEngineLoop.Tick();
			}
#if STATS
			// Write all stats for this frame out
			GStatManager.AdvanceFrame();

			if(GIsThreadedRendering)
			{
				ENQUEUE_UNIQUE_RENDER_COMMAND(AdvanceFrame,{GStatManager.AdvanceFrameForThread();});
			}
#endif
		}
	}
	return ErrorLevel;
}

#if _WINDOWS
/**
 * Moved creation of the named mutex into a function so that the MutexName
 * array wasn't on the stack during the entire run of the game
 */
HANDLE MakeNamedMutex()
{
	TCHAR MutexName[MAX_SPRINTF]=TEXT("");
	appSprintf(MutexName, TEXT("UnrealEngine3_%d"), GAMENAME);
	return CreateMutex( NULL, TRUE, MutexName);
}

/**
 * Handler for crt parameter validation. Triggers error
 *
 * @param Expression - the expression that failed crt validation
 * @param Function - function which failed crt validation
 * @param File - file where failure occured
 * @param Line - line number of failure
 * @param Reserved - not used
 */
void InvalidParameterHandler(const TCHAR* Expression,
							 const TCHAR* Function, 
							 const TCHAR* File, 
							 UINT Line, 
							 uintptr_t Reserved)
{
	// Temp workaround to fix crash in release.
#if 1
	appErrorf(TEXT("SECURE CRT: Invalid parameter detected."));
#else
	appErrorf(TEXT("SECURE CRT: Invalid parameter detected. Expression: %s Function: %s. File: %s Line: %d\n"), 
		Expression ? Expression : TEXT("Unknown"), 
		Function ? Function : TEXT("Unknown"), 
		File ? File : TEXT("Unknown"), 
		Line );
#endif
}

INT WINAPI WinMain( HINSTANCE hInInstance, HINSTANCE hPrevInstance, char*, INT nCmdShow )
{
	// all crt validation should trigger the callback
	_set_invalid_parameter_handler(InvalidParameterHandler);

#ifdef _DEBUG
	// Disable the message box for assertions and just write to debugout instead
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
	// don't fill buffers with 0xfd as we make assumptions for FNames st we only use a fraction of the entire buffer
	_CrtSetDebugFillThreshold( 0 );
#endif

	// Initialize all timing info
	appInitTiming();
	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	INT ErrorLevel			= 0;
	GIsStarted				= 1;
	hInstance				= hInInstance;
	const TCHAR* CmdLine	= GetCommandLine();
	
#if !SHIPPING_PC_GAME
	// Named mutex we use to figure out whether we are the first instance of the game running. This is needed to e.g.
	// make sure there is no contention when trying to save the shader cache.
	HANDLE NamedMutex = MakeNamedMutex();
	if( NamedMutex 
	&&	GetLastError() != ERROR_ALREADY_EXISTS 
	&& !ParseParam(CmdLine,TEXT("NEVERFIRST")) )
	{
		// We're the first instance!
		GIsFirstInstance = TRUE;
	}
	else
	{
		// There is already another instance of the game running.
		GIsFirstInstance = FALSE;
		NamedMutex = NULL;
	}
#endif

#ifdef _DEBUG
	if( TRUE )
#else
	if( IsDebuggerPresent() )
#endif
	{
		// Don't use exception handling when a debugger is attached to exactly trap the crash. This does NOT check
		// whether we are the first instance or not!
		ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
	}
	else
	{
		// Use structured exception handling to trap any crashs, walk the the stack and display a crash dialog box.
		__try
		{
			GIsGuarded = 1;
			// Run the guarded code.
			ErrorLevel = GuardedMain( CmdLine, hInInstance, hPrevInstance, nCmdShow );
			GIsGuarded = 0;
		}
		__except( CreateMiniDump( GetExceptionInformation() ) )
		{
#if !SHIPPING_PC_GAME
			// Release the mutex in the error case to ensure subsequent runs don't find it.
			ReleaseMutex( NamedMutex );
			NamedMutex = NULL;
#endif
			// Crashed.
			ErrorLevel = 1;
			GError->HandleError();
			StaticShutdownAfterError();
		}
	}

	// Final shut down.
	appExit();
#if !SHIPPING_PC_GAME
	// Release the named mutex again now that we are done.
	ReleaseMutex( NamedMutex );
	NamedMutex = NULL;
#endif

	// pause if we should
	if (GShouldPauseBeforeExit)
	{
		Sleep(INFINITE);
	}
	
	GIsStarted = 0;
	return ErrorLevel;
}


#elif PLATFORM_UNIX

static void BuildUnixCommandLine(FString &CmdLineStr, int argc, char **argv)
{
	CmdLineStr.Empty();
	if (argc >= 2)
	{
		CmdLineStr = UTF8_TO_TCHAR(argv[1]);
	}

	for (int i = 2; i < argc; i++)
	{
		CmdLineStr += TEXT(" ");
		CmdLineStr += UTF8_TO_TCHAR(argv[i]);
	}
}

int main(int argc, char **argv)
{
	GArgc = argc;
	GArgv = argv;

	// Initialize all timing info
	appInitTiming();
	// default to no game
	appStrcpy(GGameName, TEXT("None"));

	int ErrorLevel			= 0;
	GIsStarted				= 1;
	
	FString CmdLine;
	BuildUnixCommandLine(CmdLine, argc, argv);
	ErrorLevel = GuardedMain( *CmdLine, NULL, NULL, 0 );

	// Final shut down.
	appExit();

	// pause if we should
	if (GShouldPauseBeforeExit)
	{
		sleep(0xFFFFFFFF);
	}

	GIsStarted = 0;
	return ErrorLevel;
}
#endif

#endif
