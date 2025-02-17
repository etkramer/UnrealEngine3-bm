//------------------------------------------------------------------------------
// Allows the console to be redirected to the Win32 command console window that
// the application was launched from if it was launched via the .COM launcher
// application.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxConsoleWin32CommandPrompt_H__
#define FxConsoleWin32CommandPrompt_H__

#ifdef WIN32

#include "FxPlatform.h"
#include "FxConsole.h"

namespace OC3Ent
{

namespace Face
{

// The console as attached to the Win32 command prompt.
class FxConsoleWin32CommandPrompt : public FxConsoleView
{
public:
	// Constructor.
	FxConsoleWin32CommandPrompt();
	// Destructor.
	virtual ~FxConsoleWin32CommandPrompt();

	// Connects to the pipe set up in the launcher application.
	FxBool Connect( void );
	// Disconnects from the pipe set up in the launcher application.
	FxBool Disconnect( void );

	// Append HTML text to the view.
	virtual void Append( const FxString& source );
	virtual void AppendW( const FxWString& source );

protected:
	// The handle to the console.
	HANDLE _consoleHandle;
};

} // namespace Face

} // namespace OC3Ent

#endif

#endif
