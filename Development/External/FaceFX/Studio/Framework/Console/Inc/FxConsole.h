//------------------------------------------------------------------------------
// Provides base platform-independent console and HTML logging capability.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxConsole_H__
#define FxConsole_H__

#include "FxPlatform.h"
#include "FxClass.h"
#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

#define MAX_BUFFER_LENGTH 1024 * 100

#define FxMsg ::OC3Ent::Face::FxConsole::LogInfo
#define FxEcho ::OC3Ent::Face::FxConsole::LogEcho
#define FxWarn ::OC3Ent::Face::FxConsole::LogWarning
#define FxError ::OC3Ent::Face::FxConsole::LogError
#define FxCritical ::OC3Ent::Face::FxConsole::LogCritical

#define FxMsgW ::OC3Ent::Face::FxConsole::LogInfoW
#define FxEchoW ::OC3Ent::Face::FxConsole::LogEchoW
#define FxWarnW ::OC3Ent::Face::FxConsole::LogWarningW
#define FxErrorW ::OC3Ent::Face::FxConsole::LogErrorW
#define FxCriticalW ::OC3Ent::Face::FxConsole::LogCriticalW

enum FxLogType
{
	LT_Information, // An information message.
	LT_Echo,        // A command echo.
	LT_Warning,		// A warning message.
	LT_Error,		// An error message.
	LT_Critical,	// A critical error message.
};

// An abstract console view.
class FxConsoleView
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxConsoleView);
public:
	FxConsoleView()
	{
	}
	virtual ~FxConsoleView()
	{
	}
	// Append HTML text to the view.
	virtual void Append( const FxString& source ) = 0;
	virtual void AppendW( const FxWString& source ) = 0;
};

// The FaceFx console.
class FxConsole
{
	// Disable copy construction and assignment.
	FX_NO_COPY_OR_ASSIGNMENT(FxConsole);
public:
	// Constructor.
	FxConsole();
	// Destructor.
	virtual ~FxConsole();
	
	// Returns the instance.
	static FxConsole* Instance( void );

	// Sets the console instance.  This allows code in a DLL to print to the
	// same console view and file as the main application.  This will only set
	// the instance if the instance in the DLL is NULL.  When doing this
	// never call Initialize() or Destroy() from within the DLL.
	static void SetInstance( FxConsole* pInstance );
    
	// Initializes the console.
	static void Initialize( const FxString& logDirectory,
		                    const FxString& logFilename,
							FxConsoleView* pConView = NULL );

	// Destroys the console.
	static void Destroy( void );

	// Returns the log directory.
	static FxString GetLogDirectory( void );
	// Returns the full path to the log file.
	static FxString GetLogFilePath( void );

	// Shortcut to log information.
	static void LogInfo( const FxChar* format, ... );
	static void LogInfoW( const FxWChar* format, ... );
	// Shortcut to echo commands.
	static void LogEcho( const FxChar* format, ... );
	static void LogEchoW( const FxWChar* format, ... );
	// Shortcut to log warnings.
	static void LogWarning( const FxChar* format, ... );
	static void LogWarningW( const FxWChar* format, ... );
	// Shortcut to log errors.
	static void LogError( const FxChar* format, ... );
	static void LogErrorW( const FxWChar* format, ... );
	// Shortcut to log criticals.
	static void LogCritical( const FxChar* format, ... );
	static void LogCriticalW( const FxWChar* format, ... );

protected:
	// The one and only FxConsole.
	static FxConsole* _pInst;

	// The console view.
	FxConsoleView* _pConView;

	// The HTML log file.
	std::ofstream* _htmlLog;

	// The directory for the main log file.
	FxString _logDir;
	// The full path to the log file.
	FxString _logFilePath;

	// The buffer for formatting.
	static FxChar  _buffer[MAX_BUFFER_LENGTH];
	static FxWChar _bufferW[MAX_BUFFER_LENGTH];

	// The color for warning messages.
	FxString _warningColor;
	FxWString _warningColorW;
	// The color for error messages.
	FxString _errorColor;
	FxWString _errorColorW;
	// The color for critical messages.
	FxString _criticalColor;
	FxWString _criticalColorW;

	// Print string to the log and the view.
	static void _print( FxLogType msgType, const FxChar* string );
	static void _printW( FxLogType msgType, const FxWChar* string );
	static FxString _formatEcho( const FxString& string );
	static FxWString _formatEchoW( const FxWString& string );
};

} // namespace Face

} // namespace OC3Ent

#endif
