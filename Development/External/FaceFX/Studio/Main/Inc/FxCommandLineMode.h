//------------------------------------------------------------------------------
// Studio's command line mode functionality.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCommandLineMode_H__
#define FxCommandLineMode_H__

#ifndef __UNREAL__

#ifdef WIN32
	#include "FxConsoleWin32CommandPrompt.h"
#endif

#include "FxPlatform.h"
#include "FxStudioSession.h"
#include "FxAnalysisWidget.h"

namespace OC3Ent
{

namespace Face
{

// The command line mode.
class FxCommandLineMode
{
public:
	// Constructor.
	FxCommandLineMode();
	// Destructor.
	virtual ~FxCommandLineMode();

	// Starts up the command line mode.
	void Startup( void );
	// Shuts down the command line mode.
	void Shutdown( void );

	// Executes the command line mode.
	void Execute( const FxString& pathToExecScript );

protected:
#ifdef WIN32
	// The command prompt used to shuffle console output back to the command
	// prompt that launched the application.
	FxConsoleWin32CommandPrompt _commandPrompt;
#endif
	// The session for the command line.
	FxStudioSession _session;
	// The analysis widget for the command line.
	FxAnalysisWidget* _pAnalysisWidget;
	// FxTrue if expired.
	FxBool _isExpired;
};

} // namespace Face

} // namespace OC3Ent

#endif // __UNREAL__

#endif
