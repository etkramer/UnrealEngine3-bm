//------------------------------------------------------------------------------
// Provides base platform-independent console and HTML logging capability.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------


#include "FxConsole.h"

#include <fstream>
using std::endl;

#ifndef __UNREAL__
	#if defined(_MSC_VER) && _MSC_VER >= 1400 && !defined(POINTER_64)
		#define POINTER_64 
	#endif
#endif // __UNREAL__

#if defined(WIN32) && defined(FX_DEBUG) && !defined(__UNREAL__)
	#include <windows.h> // For OutputDebugString().
#endif

#include <cstdarg> // For va_list, va_start, and va_end.
#include <cstdio>  // For  _vsnprintf.

#ifdef WIN32
	#if _MSC_VER >= 1400		
		#define FXVSNPRINTF(buf, len, fmt, arg) _vsnprintf_s(buf, len, _TRUNCATE, fmt, arg)
		#define FXVSNWPRINTF(buf, len, fmt, arg) _vsnwprintf_s(buf, len, _TRUNCATE, fmt, arg)
	#else
		#define FXVSNPRINTF _vsnprintf
		#define FXVSNWPRINTF _vsnwprintf
	#endif
#else
	#define FXVSNPRINTF vsnprintf
	#define FXVSNWPRINTF vsnwprintf
#endif

namespace OC3Ent
{

namespace Face
{

FxChar FxConsole::_buffer[MAX_BUFFER_LENGTH] = {0};
FxWChar FxConsole::_bufferW[MAX_BUFFER_LENGTH] = {0};
FxConsole* FxConsole::_pInst = NULL;

FxConsole::FxConsole()
	: _pConView(NULL)
	, _htmlLog(new std::ofstream)
	, _logDir("")
	, _logFilePath("")
	, _warningColor("#FFFF80")
	, _warningColorW(L"#FFFF80")
	, _errorColor("#FF8080")
	, _errorColorW(L"#FF8080")
	, _criticalColor("#FFC080")
	, _criticalColorW(L"#FFC080")
{
}

FxConsole::~FxConsole()
{
}

FxConsole* FxConsole::Instance( void )
{
	if( NULL == _pInst )
	{
		_pInst = new FxConsole();
	}
	return _pInst;
}

void FxConsole::SetInstance( FxConsole* pInstance )
{
	if( NULL == _pInst )
	{
		_pInst = pInstance;
	}
}

void FxConsole::Initialize( const FxString& logDirectory, 
							const FxString& logFilename,
							FxConsoleView* pConView )
{
	Instance();
	_pInst->_pConView = pConView;
	_pInst->_logDir   = logDirectory;
	
	_pInst->_logFilePath = FxString::Concat(_pInst->_logDir, logFilename);
	FxAssert(_pInst->_htmlLog);
	_pInst->_htmlLog->open(_pInst->_logFilePath.GetData(), std::ios::out | std::ios::binary);
	_pInst->_htmlLog->clear();
	FxWChar ByteOrderMarker = 0xFEFF;
	_pInst->_htmlLog->write((FxChar*)&ByteOrderMarker, sizeof(FxWChar));
	FxWChar HtmlHeader[] = L"<html><body>";
	_pInst->_htmlLog->write((FxChar*)HtmlHeader, FxWStrlen(HtmlHeader) * sizeof(FxWChar));
}

void FxConsole::Destroy( void )
{
	if( _pInst )
	{
		FxAssert(_pInst->_htmlLog);
		_pInst->_htmlLog->close();
		delete _pInst->_htmlLog;
		_pInst->_htmlLog = NULL;
		delete _pInst;
		_pInst = NULL;
	}
}

FxString FxConsole::GetLogDirectory( void )
{
	Instance();
	return _pInst->_logDir;
}

FxString FxConsole::GetLogFilePath( void )
{
	Instance();
	return _pInst->_logFilePath;
}

void FxConsole::LogInfo( const FxChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_buffer, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNPRINTF(_buffer, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_print(LT_Information, _buffer);
}

void FxConsole::LogInfoW( const FxWChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_bufferW, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNWPRINTF(_bufferW, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_printW(LT_Information, _bufferW);
}

void FxConsole::LogEcho( const FxChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_buffer, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNPRINTF(_buffer, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_print(LT_Echo, _buffer);
}

void FxConsole::LogEchoW( const FxWChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_bufferW, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNWPRINTF(_bufferW, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_printW(LT_Echo, _bufferW);
}

void FxConsole::LogWarning( const FxChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_buffer, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNPRINTF(_buffer, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_print(LT_Warning, _buffer);
}

void FxConsole::LogWarningW( const FxWChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_bufferW, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNWPRINTF(_bufferW, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_printW(LT_Warning, _bufferW);
}

void FxConsole::LogError( const FxChar* format, ... )
{
	Instance();
	va_list args;
	FxMemset(_buffer, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNPRINTF(_buffer, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_print(LT_Error, _buffer);
}

void FxConsole::LogErrorW( const FxWChar* format, ... )
{
	Instance();
	va_list args;
	FxMemset(_bufferW, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNWPRINTF(_bufferW, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_printW(LT_Error, _bufferW);
}

void FxConsole::LogCritical( const FxChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_buffer, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNPRINTF(_buffer, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_print(LT_Critical, _buffer);
}

void FxConsole::LogCriticalW( const FxWChar* format, ... )
{
	Instance();
	va_list	args;
	FxMemset(_bufferW, 0, MAX_BUFFER_LENGTH);
	va_start(args, format);
	FXVSNWPRINTF(_bufferW, MAX_BUFFER_LENGTH-1, format, args);
	va_end(args);
	_pInst->_printW(LT_Critical, _bufferW);
}

void FxConsole::_print( FxLogType logType, const FxChar* string )
{
	Instance();
	
	FxString update = "<table border=0 width=\"100%\" cellspacing=0 " \
					  "cellpadding=0 bgcolor=";

	switch( logType )
	{
	case LT_Warning:
		update = FxString::Concat(update, _pInst->_warningColor);
		break;
	case LT_Error:
		update = FxString::Concat(update, _pInst->_errorColor);
		break;
	case LT_Critical:
		update = FxString::Concat(update, _pInst->_criticalColor);
		break;
	default:
		update = FxString::Concat(update, "#ffffff");
	}

	update = FxString::Concat(update, "><tr><td>");
	if( LT_Echo != logType )
	{
		if( logType == LT_Information )
		{
			update = FxString::Concat(update, "<font color=#008000>");
		}
		update = FxString::Concat(update, "// ");
	}
	if( LT_Echo == logType )
	{
		update = FxString::Concat(update, _formatEcho(string));
	}
	else
	{
		update = FxString::Concat(update, string);
	}
	if( LT_Echo != logType )
	{
		if( logType == LT_Information )
		{
			update = FxString::Concat(update, "</font>");
		}
	}
	update = FxString::Concat(update, "</td></tr></table>\n");

	FxWString temp(update.GetData());
	FxAssert(_pInst->_htmlLog);
	_pInst->_htmlLog->write((FxChar*)temp.GetData(), temp.Length() * sizeof(FxWChar));

	if( _pInst->_pConView )
	{
		_pInst->_pConView->Append(update);
	}

	// On Win32 machines, output an html stripped version to the debugger via
	// OutputDebugString().
#if defined(WIN32) && defined(FX_DEBUG) && !defined(__UNREAL__)
	FxChar output[1024] = {0};
	FxSize toWrite = 0;
	// Strip out all of the HTML tags.
	FxBool inTag = FxFalse;
	for( FxSize i = 0; i < update.Length(); ++i ) 
	{
		// Prevent this loop from overrunning the output buffer.
		if( i > 1023 )
		{
			break;
		}

		if( !inTag )
		{
			if( update[i] == '<' )
			{
				inTag = FxTrue;
			}
			else
			{
				output[toWrite] = update[i];
				toWrite++;
			}
		}
		else
		{
			if( update[i] == '>' )
			{
				inTag = FxFalse;
			}
		}
	}
	output[toWrite] = '\0';
	::OutputDebugStringA(output);
#endif
	
	FxAssert(_pInst->_htmlLog);
	_pInst->_htmlLog->flush();
}

void FxConsole::_printW( FxLogType logType, const FxWChar* string )
{
	Instance();

	FxWString update = L"<table border=0 width=\"100%\" cellspacing=0 cellpadding=0 bgcolor=";

	switch( logType )
	{
	case LT_Warning:
		update = FxWString::Concat(update, _pInst->_warningColorW);
		break;
	case LT_Error:
		update = FxWString::Concat(update, _pInst->_errorColorW);
		break;
	case LT_Critical:
		update = FxWString::Concat(update, _pInst->_criticalColorW);
		break;
	default:
		update = FxWString::Concat(update, L"#ffffff");
	}

	update = FxWString::Concat(update, L"><tr><td>");
	if( LT_Echo != logType )
	{
		if( logType == LT_Information )
		{
			update = FxWString::Concat(update, L"<font color=#008000>");
		}
		update = FxWString::Concat(update, L"// ");
	}
	if( LT_Echo == logType )
	{
		update = FxWString::Concat(update, _formatEchoW(string));
	}
	else
	{
		update = FxWString::Concat(update, string);
	}
	if( LT_Echo != logType )
	{
		if( logType == LT_Information )
		{
			update = FxWString::Concat(update, L"</font>");
		}
	}
	update = FxWString::Concat(update, L"</td></tr></table>\n");

	FxAssert(_pInst->_htmlLog);
	_pInst->_htmlLog->write((FxChar*)update.GetData(), update.Length() * sizeof(FxWChar));

	if( _pInst->_pConView )
	{
		_pInst->_pConView->AppendW(update);
	}

	// On Win32 machines, output an html stripped version to the debugger via
	// OutputDebugString().
#if defined(WIN32) && defined(FX_DEBUG) && !defined(__UNREAL__)
	FxWChar output[1024] = {0};
	FxSize toWrite = 0;
	// Strip out all of the HTML tags.
	FxBool inTag = FxFalse;
	for( FxSize i = 0; i < update.Length(); ++i ) 
	{
		// Prevent this loop from overrunning the output buffer.
		if( i > 1023 )
		{
			break;
		}

		if( !inTag )
		{
			if( update[i] == '<' )
			{
				inTag = FxTrue;
			}
			else
			{
				output[toWrite] = update[i];
				toWrite++;
			}
		}
		else
		{
			if( update[i] == '>' )
			{
				inTag = FxFalse;
			}
		}
	}
	output[toWrite] = '\0';
	::OutputDebugStringW(output);
#endif

	FxAssert(_pInst->_htmlLog);
	_pInst->_htmlLog->flush();
}

FxString FxConsole::_formatEcho( const FxString& echo )
{
	FxString retval = "<b><font color=#0000ff>";
	FxSize len = echo.Length();
	FxBool foundFirstSpace = FxFalse;
	FxBool inParameter = FxFalse;
	FxBool inString = FxFalse;
	for( FxSize i = 0; i < len; ++i )
	{
		if( (echo[i] == ' ' || echo[i] == ';') && !foundFirstSpace )
		{
			retval = FxString::Concat(retval, "</font></b>");
			foundFirstSpace = FxTrue;
		}
		if( echo[i] == '-' && !inParameter )
		{
			retval = FxString::Concat(retval, "<b><font color=#580000>");
			inParameter = FxTrue;
		}
		if( echo[i] == ' ' && inParameter )
		{
			retval = FxString::Concat(retval, "</font></b>");
			inParameter = FxFalse;
		}
		if( echo[i] == '"' && !inString )
		{
			retval = FxString::Concat(retval, "<em>");
			inString = FxTrue;
		}
		else if( echo[i] == '"' && inString )
		{
			retval = FxString::Concat(retval, "</em>");
			inString = FxFalse;
		}
		retval += echo[i];
	}
	
	// Finish up.
	if( !foundFirstSpace )
	{
		retval = FxString::Concat(retval, "</font></b>");
	}
	else if( inParameter )
	{
		retval = FxString::Concat(retval, "</font></b>");
	}
	else if( inString )
	{
		retval = FxString::Concat(retval, "</em>");
	}

	return retval;
}

FxWString FxConsole::_formatEchoW( const FxWString& echo )
{
	FxWString retval = L"<b><font color=#0000ff>";
	FxSize len = echo.Length();
	FxBool foundFirstSpace = FxFalse;
	FxBool inParameter = FxFalse;
	FxBool inString = FxFalse;
	for( FxSize i = 0; i < len; ++i )
	{
		if( (echo[i] == ' ' || echo[i] == ';') && !foundFirstSpace )
		{
			retval = FxWString::Concat(retval, L"</font></b>");
			foundFirstSpace = FxTrue;
		}
		if( echo[i] == '-' && !inParameter )
		{
			retval = FxWString::Concat(retval, L"<b><font color=#580000>");
			inParameter = FxTrue;
		}
		if( echo[i] == ' ' && inParameter )
		{
			retval = FxWString::Concat(retval, L"</font></b>");
			inParameter = FxFalse;
		}
		if( echo[i] == '"' && !inString )
		{
			retval = FxWString::Concat(retval, L"<em>");
			inString = FxTrue;
		}
		else if( echo[i] == '"' && inString )
		{
			retval = FxWString::Concat(retval, L"</em>");
			inString = FxFalse;
		}
		retval += echo[i];
	}

	// Finish up.
	if( !foundFirstSpace )
	{
		retval = FxWString::Concat(retval, L"</font></b>");
	}
	else if( inParameter )
	{
		retval = FxWString::Concat(retval, L"</font></b>");
	}
	else if( inString )
	{
		retval = FxWString::Concat(retval, L"</em>");
	}

	return retval;
}

} // namespace Face

} // namespace OC3Ent
