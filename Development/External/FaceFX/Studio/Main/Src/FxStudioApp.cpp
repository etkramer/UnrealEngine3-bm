//------------------------------------------------------------------------------
// The FaceFx Studio application.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxStudioApp.h"
#include "FxUtilityFunctions.h"

#ifdef __UNREAL__
	#include "UnrealEd.h"
#else
    #include "FxSDK.h"
    #include "FxVM.h"
    #include "FxCommandLineMode.h"
    #include "FxSecurityWrapper.h"
    
    #include "wx/cmdline.h"
    #include "wx/splash.h"
    #include "wx/debugrpt.h"
    #include "wx/net/email.h"
#endif // __UNREAL__

namespace OC3Ent
{

namespace Face
{

// Static initialization.
FxStudioMainWin* FxStudioApp::_mainWindow = NULL;
FxBool FxStudioApp::_isCommandLineMode = FxFalse;
FxBool FxStudioApp::_isSafeMode = FxFalse;
FxBool FxStudioApp::_isUnattended = FxFalse;
wxFileName FxStudioApp::_appDirectory;
wxString FxStudioApp::_pathToActor = wxEmptyString;
FxBool FxStudioApp::_showSplash = FxTrue;
FxBool FxStudioApp::_expertMode = FxFalse;

#ifndef __UNREAL__
// A simple struct to handle shutting down the FaceFX SDK in the event of a 
// failed initialization attempt.
struct FxSDKBomb
{
	FxSDKBomb() : _fuseLit(FxTrue) {}
	~FxSDKBomb()
	{
		if( _fuseLit )
		{
			FxSDKShutdown();
		}
	}
	void Defuse() { _fuseLit = FxFalse; }

private:
	FxBool _fuseLit;
};

bool FxStudioApp::OnInit( void )
{
#if !defined(FX_DEBUG) && !defined(MOD_DEVELOPER_VERSION)
	// Have wxWindows trap fatal exceptions in the application only in Release
	// builds.  In Debug builds we want to break in the debugger rather than
	// popping up a crash report dialog.
	wxHandleFatalExceptions();
#endif

	// Startup the FaceFX SDK.
	FxMemoryAllocator studioAllocator;
	studioAllocator.bUseMemoryPools = FxFalse;
	FxSDKStartup(studioAllocator);
	// Create a sdk bomb, so any exit point other than the success path results
	// in the sdk being properly shut down.
	FxSDKBomb sdkBomb;

	SetVendorName(_("OC3 Entertainment, Inc."));
	SetAppName(APP_TITLE);
	_appDirectory.AssignCwd();


	// Check if we're licensed.
	FxBool isLicensed = FxTrue;
	FxIsLicensed(isLicensed);
	if ( !isLicensed )
	{
		if( !FxDisplayAdapterSelectionDialog(NULL) )
		{
			// Quit out of the app.
			return false;
		}
	}

	FxString test("Four score and seven years ago");
	FxCheckLargeNumber(test);

	// Command line parsing is done in wxApp::OnInit().
    if( !Super::OnInit() )
	{
        return false;
	}

	// Create the main window.
	_mainWindow = new FxStudioMainWin(_expertMode);
    SetTopWindow(_mainWindow);

	// If we're past our expiration date, bail.
	FxAnalysisLicenseeInfo licenseeInfo;
	if( _mainWindow->IsExpired(licenseeInfo) )
	{
		wxString errorMsg = wxString::Format(_("Sorry... Your evaluation period has expired.  If you would like to continue using FaceFX,\nplease contact OC3 Entertainment by sending an email to support@oc3ent.com.\n\nLicensed to: %s\nProject: %s\nExpires: %s"),
			wxString::FromAscii(licenseeInfo.licenseeName),
			wxString::FromAscii(licenseeInfo.licenseeProject),
			wxString::FromAscii(licenseeInfo.expirationDate));
		FxMessageBox(errorMsg, _("Thank You for Evaluating FaceFX"), wxOK | wxICON_STOP);
		FxVM::ExecuteCommand("exit");
		return false;
	}

	// Show the splash screen.
	wxSplashScreen* splash = NULL;
	if( _showSplash )
	{
		wxBitmap bitmap;
		if( bitmap.LoadFile(FxStudioApp::GetBitmapPath(wxT("Splash.bmp")), wxBITMAP_TYPE_BMP) )
		{
			splash = new wxSplashScreen(bitmap,
				wxSPLASH_CENTRE_ON_SCREEN|wxSPLASH_TIMEOUT,
				1500, NULL, -1, wxDefaultPosition, wxDefaultSize,
				wxSIMPLE_BORDER|wxSTAY_ON_TOP);
		}
		wxSafeYield();
	}

	// Sleep for a second to let the splash screen have a little fun time alone.
	wxSleep(1);

    _mainWindow->Show();
    _mainWindow->Layout();
	
	// Automatically execute the autoexec.fxl script if it exists.
	// Give the main window a little fun time alone before executing.
	wxSleep(1);
	FxTriggerScriptEvent(STET_PostLoadApp);
	

	//@todo Load the actor specified on the command line.
	// This needs to happen after the first layout or else the app looks really,
	// really weird.
//	wxString actorPath = GetPathToActor();
//	if( actorPath != wxEmptyString )
//	{
//		wxString command = wxString::Format(wxT("loadActor -f \"%s\""), actorPath);
//		FxVM::ExecuteCommand(FxString(command.mb_str(wxConvLibc)));
//	}

	_mainWindow->LoadOptions();
	sdkBomb.Defuse();
    return true;
}

int FxStudioApp::OnExit( void )
{
	// Shutdown the FaceFX SDK.
	FxSDKShutdown();

	return Super::OnExit();
}

// FaceFX Studio compressed crash report sent via email to crash@oc3ent.com.
class FxCrashReport : public wxDebugReportCompress
{
public:
	virtual wxString GetReportName() const
	{
		return wxT("FaceFxStudio");
	}
	virtual bool DoProcess( void )
	{
		if( !wxDebugReportCompress::DoProcess() )
			return false;

		wxMailMessage msg(GetReportName(),
						  wxT("crash@oc3ent.com"),
						  wxEmptyString, // Empty message body.
						  wxEmptyString, // Empty From Address.
						  GetCompressedFileName(),
						  wxT("crashreport.zip"));

		return wxEmail::Send(msg);
	}
};

void FxStudioApp::OnFatalException( void )
{
	// Create a new debug report.
	FxCrashReport* pReport = new FxCrashReport();
	if( pReport )
	{
		// Add all of the standard debug report files (mini-dump .dmp file and an
		// XML file with system info and a stack trace).
		pReport->AddAll(wxDebugReport::Context_Exception);

		// Add the FaceFX Studio log file to the report.
		wxFileName logPath(_appDirectory);
		logPath.AppendDir(wxT("Logs"));
		logPath.SetName(wxT("FaceFxStudioLog.html"));
		pReport->AddFile(logPath.GetFullPath(), wxT("FaceFX Studio Log File"));

		// Ask the user if they would like to submit the crash report.
		if( wxDebugReportPreviewStd().Show(*pReport) )
		{
			// If the user canceled submitting the crash report, we won't get in
			// here.
			if( pReport->Process() )
			{
				pReport->Reset();
			}
		}

		delete pReport;
		pReport = NULL;
	}
}
#endif // __UNREAL__

wxWindow* FxStudioApp::GetMainWindow( void )
{
	return _mainWindow;
}

#ifdef __UNREAL__
void FxStudioApp::SetMainWindow( wxWindow* pMainWindow )
{
	_mainWindow = static_cast<FxStudioMainWin*>(pMainWindow);
	// Automatically execute the autoexec.fxl script if it exists.
	GetAppDirectory();
	wxFileName autoexecPath(_appDirectory);
	autoexecPath.SetName(wxT("autoexec.fxl"));
	FxString autoexecFile(autoexecPath.GetFullPath().mb_str(wxConvLibc));
	if( FxFileExists(autoexecFile) )
	{
		// Give the main window a little fun time alone before executing.
		wxSleep(1);
		FxString execCommand = "exec -file \"";
		execCommand += autoexecFile;
		execCommand += "\"";
		FxVM::ExecuteCommand(execCommand);
	}
}
#endif // __UNREAL__

FxBool FxStudioApp::IsCommandLineMode( void )
{
	return _isCommandLineMode;
}

FxBool FxStudioApp::IsSafeMode( void )
{
	return _isSafeMode;
}

void FxStudioApp::SetIsUnattended( FxBool isUnattended )
{
	_isUnattended = isUnattended;
}

FxBool FxStudioApp::IsUnattended( void )
{
	return _isUnattended;
}

#ifdef __UNREAL__
void FxStudioApp::CheckForSafeMode( void )
{
	// If the user is holding the shift key, we should start up
	// in safe mode.
	if( wxGetKeyState(WXK_SHIFT) )
	{
		_isSafeMode = FxTrue;
	}
	else
	{
		_isSafeMode = FxFalse;
	}
}
#endif // __UNREAL__

wxString FxStudioApp::GetPathToActor( void )
{
	return _pathToActor;
}

const wxFileName& FxStudioApp::GetAppDirectory( void )
{
#ifdef __UNREAL__
	_appDirectory = wxString(appBaseDir());
	_appDirectory.AppendDir(wxString(wxT("FaceFX")));
#endif // __UNREAL__
	return _appDirectory;
}

wxString FxStudioApp::GetIconPath(const wxString& iconFileName)
{
	GetAppDirectory();
	wxFileName retval(_appDirectory);
	retval.AppendDir(wxT("res"));
	retval.AppendDir(wxT("icons"));
	retval.SetName(iconFileName);
	return retval.GetFullPath();
}

wxString FxStudioApp::GetBitmapPath(const wxString& bitmapFileName)
{
	GetAppDirectory();
	wxFileName retval(_appDirectory);
	retval.AppendDir(wxT("res"));
	retval.AppendDir(wxT("bmp"));
	retval.SetName(bitmapFileName);
	return retval.GetFullPath();
}

wxString FxStudioApp::GetDataPath( void )
{
	GetAppDirectory();
	wxFileName retval(_appDirectory);
	retval.SetFullName(wxT("FonixData.cdf"));
	return retval.GetFullPath();
}

#ifndef __UNREAL__
void FxStudioApp::OnInitCmdLine( wxCmdLineParser& parser )
{
    Super::OnInitCmdLine(parser);
    static const wxCmdLineEntryDesc cmdLineDesc[] = {
		{ wxCMD_LINE_OPTION,
		  wxT("exec"), 
		  wxT("exec"), 
		  _("Executes a script in command line mode."), 
		},
		{ wxCMD_LINE_SWITCH,
		  wxT("nosplash"),
		  wxT("nosplash"),
		  _("Disables the splash screen on application startup."),
		  wxCMD_LINE_VAL_NONE,
		  wxCMD_LINE_PARAM_OPTIONAL
		},
		{ wxCMD_LINE_SWITCH,
		  wxT("expert"),
		  wxT("expert"),
		  _("Starts FaceFX Studio in expert mode."),
		  wxCMD_LINE_VAL_NONE,
		  wxCMD_LINE_PARAM_OPTIONAL
		},
		{ wxCMD_LINE_SWITCH,
		  wxT("safe"),
		  wxT("safe"),
		  _("Starts FaceFX Studio in safe mode."),
		  wxCMD_LINE_VAL_NONE,
		  wxCMD_LINE_PARAM_OPTIONAL
		},
		/*{ wxCMD_LINE_OPTION,
		  wxT("actor"),
		  wxT("actor"),
		  _("Load FaceFX Studio with an actor.")
		},*/
        { wxCMD_LINE_NONE }
    };
    parser.SetDesc(cmdLineDesc);
}

bool FxStudioApp::OnCmdLineParsed( wxCmdLineParser& parser )
{
    Super::OnCmdLineParsed(parser);

	// Batch mode takes precedence over all other command line options or
	// switches.
	wxString pathToExecScript = wxEmptyString;
	if( parser.Found(wxT("exec"), &pathToExecScript) )
	{
		// Start up the command line mode.
		_isCommandLineMode = FxTrue;
		_isUnattended      = FxTrue;
		FxCommandLineMode commandLineMode;
		commandLineMode.Startup();

		// Run command line mode.
		FxString execScriptPath(pathToExecScript.mb_str(wxConvLibc));
		commandLineMode.Execute(execScriptPath);

		// Shutdown the command line mode.
		commandLineMode.Shutdown();		
		return false;
	}
	else
	{
		if( parser.Found(wxT("nosplash")) )
		{
			_showSplash = FxFalse;
		}
		if( parser.Found(wxT("expert")) )
		{
			_expertMode = FxTrue;
		}
		if( parser.Found(wxT("safe")) )
		{
			_isSafeMode = FxTrue;
		}
		/*if( !parser.Found(wxT("actor"), &_pathToActor) )
		{
			_pathToActor = wxEmptyString;
		}*/
	}

#ifdef WIN32
	FxConsoleWin32CommandPrompt commandPrompt;
	commandPrompt.Connect();
	commandPrompt.Disconnect();
#endif

    return true;
}

IMPLEMENT_APP(FxStudioApp)
#endif // __UNREAL__

} // namespace Face

} // namespace OC3Ent
