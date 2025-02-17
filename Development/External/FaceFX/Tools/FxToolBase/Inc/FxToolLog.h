//------------------------------------------------------------------------------
// Generic log file support for all FaceFx plug-ins.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxToolLog_H__
#define FxToolLog_H__

#include "FxString.h"

namespace OC3Ent
{

namespace Face
{

#define FxToolLog ::OC3Ent::Face::FxToolLogFile::Log

class FxToolLogFile
{
public:
	// Sets the full path to the log file.
	static void SetPath( const FxString& logPath );
	// Returns the full path to the log file.
	static FxString GetPath( void );

	// Sets whether the log information is also echoed to cout.
	static void SetEchoToCout( FxBool echoToCout );
	// Returns whether the log information is also echoed to cout.
	static FxBool GetEchoToCout( void );

	// Printf-style log output.
	static void Log( const FxChar* format, ... );

protected:
	// The full path to the log file.
	static FxString _logPath;
	// FxTrue if the log information is also echoed to cout; FxFalse otherwise.
	static FxBool _echoToCout;
};

} // namespace Face

} // namespace OC3Ent

#endif
