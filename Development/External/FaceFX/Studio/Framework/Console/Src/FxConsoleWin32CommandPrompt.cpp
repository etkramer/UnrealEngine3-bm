//------------------------------------------------------------------------------
// Allows the console to be redirected to the Win32 command console window that
// the application was launched from if it was launched via the .COM launcher
// application.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

// This technique is from the CodeGuru article located here:
// http://www.codeguru.com/Cpp/W-D/console/redirection/article.php/c3955/

#include "stdwx.h"
#include "FxConsoleWin32CommandPrompt.h"

#include <iostream>
using std::cout;

#ifdef WIN32

namespace OC3Ent
{

namespace Face
{

FxConsoleWin32CommandPrompt::FxConsoleWin32CommandPrompt()
{
	_consoleHandle = INVALID_HANDLE_VALUE;
}

FxConsoleWin32CommandPrompt::~FxConsoleWin32CommandPrompt()
{
	if( INVALID_HANDLE_VALUE != _consoleHandle )
	{
		FlushFileBuffers(_consoleHandle);
		CloseHandle(_consoleHandle);
		_consoleHandle = INVALID_HANDLE_VALUE;
	}
}

FxBool FxConsoleWin32CommandPrompt::Connect( void )
{
	if( INVALID_HANDLE_VALUE == _consoleHandle )
	{
		// Connect to the named pipe set up in the launcher application.
		TCHAR PipeName[256];
		_stprintf(PipeName, _T("\\\\.\\pipe\\%dcout"), GetCurrentProcessId());
		_consoleHandle = CreateFile(PipeName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
	}
	return INVALID_HANDLE_VALUE != _consoleHandle;
}

FxBool FxConsoleWin32CommandPrompt::Disconnect( void )
{
	if( INVALID_HANDLE_VALUE != _consoleHandle )
	{
		// Signal the die event to the launcher application.
		TCHAR dieEvent[256];
		_stprintf(dieEvent, _T("launcher_die_event%d"), GetCurrentProcessId());
		HANDLE hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, dieEvent);
		if( NULL != hEvent )
		{
			SetEvent(hEvent);
			CloseHandle(hEvent);
		}
		CloseHandle(_consoleHandle);
		_consoleHandle = INVALID_HANDLE_VALUE;
		return FxTrue;
	}
	return FxFalse;
}

void FxConsoleWin32CommandPrompt::Append( const FxString& source )
{
	if( INVALID_HANDLE_VALUE != _consoleHandle )
	{
		FxChar output[1024] = {0};
		FxSize toWrite = 0;
		// Strip out all of the HTML tags.
		FxBool inTag = FxFalse;
		for( FxSize i = 0; i < source.Length(); ++i ) 
		{
			// Prevent this loop from overrunning the output buffer.
			if( i > 1023 )
			{
				break;
			}

			if( !inTag )
			{
				if( source[i] == '<' )
				{
					inTag = FxTrue;
				}
				else
				{
					output[toWrite] = source[i];
					toWrite++;
				}
			}
			else
			{
				if( source[i] == '>' )
				{
					inTag = FxFalse;
				}
			}
		}
		FxSize length = FxStrlen(output);
		output[length] = '\0';
		FxString outputCopy(output);
		while( outputCopy.Replace("&nbsp;", " ") ) {}
		while( outputCopy.Replace("&lt",    "<") ) {}
		while( outputCopy.Replace("&gt",    ">") ) {}
		DWORD written;
		WriteFile(_consoleHandle, outputCopy.GetCstr(), outputCopy.Length(), &written, NULL);
	}
}

void FxConsoleWin32CommandPrompt::AppendW( const FxWString& source )
{
	if( INVALID_HANDLE_VALUE != _consoleHandle )
	{
		FxWChar output[1024] = {0};
		FxSize toWrite = 0;
		// Strip out all of the HTML tags.
		FxBool inTag = FxFalse;
		for( FxSize i = 0; i < source.Length(); ++i ) 
		{
			// Prevent this loop from overrunning the output buffer.
			if( i > 1023 )
			{
				break;
			}

			if( !inTag )
			{
				if( source[i] == '<' )
				{
					inTag = FxTrue;
				}
				else
				{
					output[toWrite] = source[i];
					toWrite++;
				}
			}
			else
			{
				if( source[i] == '>' )
				{
					inTag = FxFalse;
				}
			}
		}
		FxSize length = FxWStrlen(output);
		output[length] = '\0';
		FxWString outputCopy(output);
		while( outputCopy.Replace(L"&nbsp;", L" ") ) {}
		while( outputCopy.Replace(L"&lt",    L"<") ) {}
		while( outputCopy.Replace(L"&gt",    L">") ) {}
		// The DOS box cannot handle Unicode strings, so convert to ASCII and
		// pipe it through Append().
		FxString temp(outputCopy.GetCstr());
		Append(temp);
	}
}

} // namespace Face

} // namespace OC3Ent

#endif
