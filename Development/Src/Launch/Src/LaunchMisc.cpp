/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

#include "LaunchPrivate.h"

#if _WINDOWS

#include "PropertyWindowManager.h"
#include <ErrorRep.h>

#pragma comment(lib, "Faultrep.lib")

// !!! FIXME: remove this.
#if PLATFORM_UNIX
#define appShowSplash(x) STUBBED("appShowSplash")
#define appHideSplash() STUBBED("appHideSplash")
#endif

//
// Minidump support/ exception handling.
//

#pragma pack(push,8)
#include <DbgHelp.h>
#pragma pack(pop)

TCHAR MiniDumpFilenameW[1024] = TEXT("");
char  MiniDumpFilenameA[1024] = "";			// We can't use alloca before writing out minidump so we avoid the TCHAR_TO_ANSI macro

INT CreateMiniDump( LPEXCEPTION_POINTERS ExceptionInfo )
{
	// Try to create file for minidump.
	HANDLE FileHandle	= CreateFileW( MiniDumpFilenameW, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
		
	// Write a minidump.
	if( FileHandle != INVALID_HANDLE_VALUE )
	{
		MINIDUMP_EXCEPTION_INFORMATION DumpExceptionInfo;

		DumpExceptionInfo.ThreadId			= GetCurrentThreadId();
		DumpExceptionInfo.ExceptionPointers	= ExceptionInfo;
		DumpExceptionInfo.ClientPointers	= true;

		MiniDumpWriteDump( GetCurrentProcess(), GetCurrentProcessId(), FileHandle, MiniDumpNormal, &DumpExceptionInfo, NULL, NULL );
		CloseHandle( FileHandle );
	}

	const SIZE_T StackTraceSize = 65535;
	ANSICHAR* StackTrace = (ANSICHAR*) appSystemMalloc( StackTraceSize );
	StackTrace[0] = 0;
	// Walk the stack and dump it to the allocated memory.
	appStackWalkAndDump( StackTrace, StackTraceSize, 0, ExceptionInfo->ContextRecord );
	appStrncat( GErrorHist, ANSI_TO_TCHAR(StackTrace), ARRAY_COUNT(GErrorHist) - 1 );
	appSystemFree( StackTrace );

#if SHIPPING_PC_GAME 
	DWORD dwOpt = 0;
	EFaultRepRetVal repret = ReportFault( ExceptionInfo, dwOpt);
#endif

	return EXCEPTION_EXECUTE_HANDLER;
}

//
//	WxLaunchApp implementation.
//

/**
 * Gets called on initialization from within wxEntry.	
 */
bool WxLaunchApp::OnInit()
{
	wxApp::OnInit();

	if( ParseParam(appCmdLine(),TEXT("NOSPLASH")) != TRUE )
	{
		FString SplashPath;
		// make sure a splash was found
		if (appGetSplashPath(GIsEditor ? TEXT("PC\\EdSplash.bmp") : TEXT("PC\\Splash.bmp"), SplashPath) == TRUE)
		{
			appShowSplash( *SplashPath );
		}
	}

	// Initialize XML resources
	wxXmlResource::Get()->InitAllHandlers();
	verify( wxXmlResource::Get()->Load( TEXT("wxRC/UnrealEd*.xrc") ) );

	INT ErrorLevel = GEngineLoop.Init();
	if ( ErrorLevel )
	{
		appHideSplash( );
		return 0;
	}

	// Init subsystems
	InitPropertySubSystem();

	if ( !GIsEditor )
	{
		appHideSplash( );
	}

	return 1;
}

/** 
 * Gets called after leaving main loop before wxWindows is shut down.
 */
int WxLaunchApp::OnExit()
{
	return wxApp::OnExit();
}

/**
 * Performs any required cleanup in the case of a fatal error.
 */
void WxLaunchApp::ShutdownAfterError()
{
}

/**
 * Callback from wxWindows main loop to signal that we should tick the engine.
 */
void WxLaunchApp::TickUnreal()
{
#if STATS
	GFPSCounter.Update(appSeconds());
#endif
	if( !GIsRequestingExit )
	{
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

/**
 * The below is a manual expansion of wxWindows's IMPLEMENT_APP to allow multiple wxApps.
 *
 * @warning: when upgrading wxWindows, make sure that the below is how IMPLEMENT_APP looks like
 */
wxAppConsole * wxCreateApp()
{
	wxApp::CheckBuildOptions(WX_BUILD_OPTIONS_SIGNATURE, "UnrealEngine3");
	return GIsEditor ? new WxUnrealEdApp : new WxLaunchApp;
}
wxAppInitializer wxTheAppInitializer((wxAppInitializerFunction) wxCreateApp);
WxLaunchApp& wxGetApp() { return *(WxLaunchApp *)wxTheApp; }

#endif
