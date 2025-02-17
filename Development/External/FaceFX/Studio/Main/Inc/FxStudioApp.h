//------------------------------------------------------------------------------
// The FaceFx Studio application.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxStudioApp_H__
#define FxStudioApp_H__

#include "FxStudioMainWin.h"

namespace OC3Ent
{

namespace Face
{

#ifdef __UNREAL__
class FxStudioApp
#else
class FxStudioApp : public wxApp
#endif // __UNREAL__
{
#ifndef __UNREAL__
    // Typedef for the parent class.   
	typedef wxApp Super;
public:
	// Called by wxWindows to initialize the application.  Returns true if
	// the initialization was successful.  Returning false will cause wxWindows
	// to stop application execution.
    virtual bool OnInit( void );
	// Called by wxWindows when the application is shutting down, after all the
	// windows have been destroyed, and just prior to wxWindows shutting itself
	// down.
	virtual int OnExit( void );
	// Called by wxWindows when a crash occurs.
	virtual void OnFatalException( void );
#else
public:
#endif // __UNREAL__

	// Returns a pointer to the main window.
	static wxWindow* GetMainWindow( void );
#ifdef __UNREAL__
	// Sets the pointer to the main window.
	static void SetMainWindow( wxWindow* pMainWindow );
#endif // __UNREAL__
	// Returns FxTrue if FaceFX Studio is in Command Line Mode.
	static FxBool IsCommandLineMode( void );
	// Returns FxTrue if FaceFX Studio was started in "safe" mode.
	static FxBool IsSafeMode( void );

	// Sets if the application should run in unattended mode.
	static void SetIsUnattended( FxBool isUnattended );
	// Returns FxTrue if the application is running in unattended mode.
    static FxBool IsUnattended( void );

#ifdef __UNREAL__
	// Checks for and sets the safe mode states.
	static void CheckForSafeMode( void );
#endif // __UNREAL__

	// Returns the path to the actor file that was loaded via the command line.
	// Note that this feature does not currently work.
	wxString FxStudioApp::GetPathToActor( void );

	// Returns the application directory.
	static const wxFileName& GetAppDirectory( void );
	// Returns an icon path.
	static wxString GetIconPath(const wxString& iconFileName);
	// Returns a bitmap path.
	static wxString GetBitmapPath(const wxString& bitmapFileName);
	// Returns the data path.
	static wxString GetDataPath( void );

protected:
#ifndef __UNREAL__
    // Called by wxWindows to initialize command line handling.
    void OnInitCmdLine( wxCmdLineParser& parser );
    // Called by wxWindows to allow FxStudioApp to handle the command line.
    bool OnCmdLineParsed( wxCmdLineParser& parser );
#endif // __UNREAL__

	// The main window.
	static FxStudioMainWin* _mainWindow;
	// Flag that is FxTrue if FaceFX Studio is in Command Line Mode.
	static FxBool _isCommandLineMode;
	// Flag that is FxTrue if FaceFX Studio is in "safe" mode.
	static FxBool _isSafeMode;
	// Flag that is FxTrue if FaceFX Studio is in unattended mode.
	static FxBool _isUnattended;

	// The application directory.
	static wxFileName _appDirectory;

	// The path to the actor
	static wxString _pathToActor;

	// A flag that specifies if the splash screen should be shown on startup.
	static FxBool _showSplash;

	// A flag that specifies if Studio should be started up in expert mode.
	static FxBool _expertMode;
};
DECLARE_APP(FxStudioApp)

} // namespace Face

} // namespace OC3Ent

#endif
