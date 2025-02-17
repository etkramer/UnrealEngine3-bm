//------------------------------------------------------------------------------
// Studio's command line mode functionality.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#ifndef __UNREAL__

#include "FxCommandLineMode.h"
#include "FxSDK.h"
#include "FxVM.h"
#include "FxAudioFile.h"
#include "FxSessionProxy.h"
#include "FxProgressDialog.h"
#include "FxAudioPlayer.h"
#include "FxAudioPlayerOpenAL.h"
#include "FxPhonemeMap.h"
#include "FxPhonWordList.h"
#include "FxFaceGraphNodeGroup.h"
#include "FxCameraManager.h"
#include "FxActorTemplate.h"
#include "FxAnimSetManager.h"
#include "FxStudioApp.h"

#if defined(WIN32) && !defined(__UNREAL__)
	#include "FxAudioPlayerDirectSound8.h"
#endif

#include "FxCGConfigManager.h"

#include "wx/filename.h"
#include "zlib.h"
#include <iostream>

namespace OC3Ent
{

namespace Face
{

FxCommandLineMode::FxCommandLineMode()
	: _pAnalysisWidget(NULL)
	, _isExpired(FxFalse)
{
}

FxCommandLineMode::~FxCommandLineMode()
{
}

void FxCommandLineMode::Startup( void )
{
	// Register the classes.
	FxStudioSession::StaticClass();
	FxAudioPlayer::StaticClass();
#ifndef __APPLE__
	FxAudioPlayerOpenAL::StaticClass();
#endif
#if defined(WIN32) && !defined(__UNREAL__)
	FxAudioPlayerDirectSound8::StaticClass();
#endif
	FxAudio::StaticClass();
	// This is required for backwards compatibility reasons.
	FxDigitalAudio::StaticClass();
	FxAnimUserData::StaticClass();
	FxFaceGraphNodeGroup::StaticClass();
	FxPhonemeMap::StaticClass();
	FxPhonWordList::StaticClass();
	FxViewportCamera::StaticClass();
	FxWorkspace::StaticClass();
	FxActorTemplate::StaticClass();
	FxAnimSetMiniSession::StaticClass();

	// Turn off the prompt to save actor feature.
	_session.SetPromptToSaveActor(FxFalse);

	// Startup the virtual machine.
	FxSessionProxy::SetSession(&_session);
    FxVM::Startup();

	// Startup the audio file system.
	FxAudioFile::Startup();

	// Startup the Studio time management system.
	FxTimeManager::Startup();

	// Startup the Studio animation player
	FxStudioAnimPlayer::Startup();

	// Startup the FxCG configuration manager.
	FxCGConfigManager::Startup();

	// Reroute the console output to the command prompt that Studio was
	// launched from.
#ifdef WIN32
	_commandPrompt.Connect();
#endif

	// Initialize the console.
	wxFileName logPath(FxStudioApp::GetAppDirectory());
	logPath.AppendDir(wxT("Logs"));
	FxString logsDirectory(logPath.GetFullPath().mb_str(wxConvLibc));
#ifdef WIN32
	FxConsole::Initialize(logsDirectory, "FaceFxStudioLog.html", &_commandPrompt);
#endif

	// Put the progress system into console mode.
	FxProgressDialog::IsConsoleMode = FxTrue;

	FxMsg( "Console initialized." );

	FxMsg( "Initializing %s version %s...", APP_TITLE.mb_str(wxConvLibc), APP_VERSION.mb_str(wxConvLibc) );
	FxMsg( "Using FaceFX SDK version %s", FxSDKGetVersionString().GetData() );
	FxMsg( "FaceFX SDK licensee: %s", FxSDKGetLicenseeName().GetData() );
	FxMsg( "FaceFX SDK licensee project: %s", FxSDKGetLicenseeProjectName().GetData() );
	wxString wxVersionString = wxVERSION_STRING;
	FxMsg( "Using %s", wxVersionString.mb_str(wxConvLibc) );
	FxMsg( "Using zlib version %s", zlibVersion() );

	FxMsg( "OS Version: %s", ::wxGetOsDescription().mb_str(wxConvLibc) );
	wxMemorySize freeMemory = ::wxGetFreeMemory();
	if( freeMemory != -1 )
	{
		FxMsg( "Memory: %d bytes free", freeMemory );
	}
	else
	{
		FxMsg( "Memory: n/a" );
	}
	FxMsg( "System: %s", ::wxGetHostName().mb_str(wxConvLibc) );
	FxMsg( "User: %s", ::wxGetUserId().mb_str(wxConvLibc) );

	FxMsg( "Initializing widgets..." );

	// Create the analysis widget.
	_pAnalysisWidget = new FxAnalysisWidget(&_session);
	_pAnalysisWidget->SetAnimControllerPointer(_session.GetAnimController());

	// Broadcast the application startup message.
	_session.OnAppStartup(NULL);

	// Startup the workspace manager.
	FxWorkspaceManager::Startup();

	// Check for expiration here.
	_isExpired = _pAnalysisWidget->IsExpired();

	FxMsg( "FaceFX Studio initialized properly." );

	// Automatically execute the autoexec.fxl script if it exists.
	wxFileName autoexecPath = FxStudioApp::GetAppDirectory();
	autoexecPath.SetName(wxT("autoexec.fxl"));
	FxString autoexecFile(autoexecPath.GetFullPath().mb_str(wxConvLibc));
	if( FxFileExists(autoexecFile) )
	{
		FxString execCommand = "exec -file \"";
		execCommand += autoexecFile;
		execCommand += "\"";
		FxVM::ExecuteCommand(execCommand);
	}
}

void FxCommandLineMode::Shutdown( void )
{
	// Broadcast the application shutdown message.
	_session.OnAppShutdown(NULL);

	// Clean up the analysis widget.
	if( _pAnalysisWidget )
	{
		delete _pAnalysisWidget;
		_pAnalysisWidget = NULL;
	}

	// Shutdown the workspace manager.
	FxWorkspaceManager::Shutdown();

	// Shutdown the FxCG configuration manager.
	FxCGConfigManager::Shutdown();

	// Shutdown the animation player
	FxStudioAnimPlayer::Shutdown();

	// Shutdown the Studio time management system.
	FxTimeManager::Shutdown();
	
	// Destroy the console.
	FxConsole::Destroy();

	// Shutdown the audio file system.
	FxAudioFile::Shutdown();

	// Shutdown the virtual machine.
	FxVM::Shutdown();

	// Disconnect from the command prompt.
#ifdef WIN32
	_commandPrompt.Disconnect();
#endif
}

void FxCommandLineMode::Execute( const FxString& pathToExecScript )
{
	// Only allow script execution if not expired.
	if( !_isExpired )
	{
		// Execute the script.
		FxString execCommand = "exec -file \"";
		execCommand = FxString::Concat(execCommand, pathToExecScript);
		execCommand = FxString::Concat(execCommand, "\"");
		FxVM::ExecuteCommand(execCommand);
	}
	else
	{
		FxError( "Sorry... Your evaluation period has expired.  If you would like to continue using FaceFX," );
		FxError( "please contact OC3 Entertainment by sending an email to support@oc3ent.com." );
	}
}

} // namespace Face

} // namespace OC3Ent

#endif // __UNREAL__

