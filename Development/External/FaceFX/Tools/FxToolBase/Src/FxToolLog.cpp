//------------------------------------------------------------------------------
// Generic log file support for all FaceFx plug-ins.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "FxToolLog.h"

#include <stdio.h>
#ifdef WIN32
	#include <stdarg.h>
#else
	#include <varargs.h>
#endif

#include <iostream>
#include <fstream>

namespace OC3Ent
{

namespace Face
{

// The full path to the log file.
FxString FxToolLogFile::_logPath;
// FxTrue if the log information is also echoed to cout; FxFalse otherwise.
FxBool FxToolLogFile::_echoToCout = FxFalse;

// Sets the full path to the log file.
void FxToolLogFile::SetPath( const FxString& logPath )
{
	_logPath = logPath;
	// Erase the contents of the log file.
	std::ofstream output(_logPath.GetData());
	FxToolLog("Logging to file %s", _logPath.GetData());
}

// Returns the full path to the log file.
FxString FxToolLogFile::GetPath( void )
{
	return _logPath;
}

// Sets whether the log information is also echoed to cout.
void FxToolLogFile::SetEchoToCout( FxBool echoToCout )
{
	_echoToCout = echoToCout;
	if( _echoToCout )
	{
		FxToolLog("Logging to cout");
	}
}

// Returns whether the log information is also echoed to cout.
FxBool FxToolLogFile::GetEchoToCout( void )
{
	return _echoToCout;
}

// Printf-style log output.
void FxToolLogFile::Log( const FxChar* format, ... )
{
	va_list	argList;
	FxChar	buffer[1024];

	va_start(argList, format);
	vsprintf(buffer, format, argList);
	va_end(argList);

	if( _logPath.Length() > 0 )
	{
		std::ofstream output(_logPath.GetData(), std::ios_base::app);
		if( output )
		{
			output << "FaceFX: " << buffer << "\r\n";
		}
	}
	if( _echoToCout )
	{
		std::cout << "FaceFX: " << buffer << std::endl;
	}
}

} // namespace Face

} // namespace OC3Ent
