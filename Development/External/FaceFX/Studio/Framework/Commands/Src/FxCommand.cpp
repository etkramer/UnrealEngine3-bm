//------------------------------------------------------------------------------
// The base class for a FaceFX Studio command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCommand.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

static const FxChar kStringArraySeparator = '|';

//------------------------------------------------------------------------------
// FxCommandArgumentDesc.
//------------------------------------------------------------------------------

FxCommandArgumentDesc::FxCommandArgumentDesc()
{
}

FxCommandArgumentDesc::
FxCommandArgumentDesc( const FxString& argNameShort,
					   const FxString& argNameLong,
					   FxCommandArgumentType argType )
					   : ArgNameShort(argNameShort)
					   , ArgNameLong(argNameLong)
					   , ArgType(argType)
{
}

//------------------------------------------------------------------------------
// FxCommandArgumentList.
//------------------------------------------------------------------------------

FxCommandArgumentList::FxCommandArgumentList()
{
}

FxCommandArgumentList::~FxCommandArgumentList()
{
}

void FxCommandArgumentList::AddArgument( const FxCommandArgument& arg )
{
	_args.PushBack(arg);
}

FxSize FxCommandArgumentList::GetNumArguments( void ) const
{
	return _args.Length();
}

const FxCommandArgument& FxCommandArgumentList::GetArgument( FxSize index ) const
{
	return _args[index];
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName ) const
{
	if( FxInvalidIndex != _getArgIndex(argName) )
	{
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxReal& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_Real )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxReal!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			arg = static_cast<FxReal>(FxAtof(_args[argIndex].Arg.GetData()));
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxDReal& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_DReal )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxDReal!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			arg = FxAtof(_args[argIndex].Arg.GetData());
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxSize& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_Size )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxSize!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			arg = FxAtoi(_args[argIndex].Arg.GetData());
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxInt32& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_Int32 )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxInt32!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			arg = FxAtoi(_args[argIndex].Arg.GetData());
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxString& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_String )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxString!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			arg = _args[argIndex].Arg;
			if( arg.Length() > 0 )
			{
				return FxTrue;	
			}
			FxVM::DisplayError("Empty String Argument!");
		}
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxArray<FxString>& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_StringArray )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxArray[FxString]!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			arg.Clear();
			FxString helperString;
			for( FxSize i = 0; i < _args[argIndex].Arg.Length(); ++i )
			{
				// There is a string separator indicating a new element.
				if( _args[argIndex].Arg[i] == kStringArraySeparator )
				{
					arg.PushBack(helperString);
					helperString.Clear();
				}
				// Fill in the helperString.
				else
				{
					helperString = FxString::Concat(helperString, _args[argIndex].Arg[i]);
				}
			}
			// Make sure the last argument is picked up.
			if( helperString.Length() > 0 )
			{
				arg.PushBack(helperString);
			}
			helperString.Clear();
			return FxTrue;
		}
	}
	return FxFalse;
}

FxBool FxCommandArgumentList::GetArgument( const FxString& argName, FxBool& arg ) const
{
	FxSize argIndex = _getArgIndex(argName);
	if( FxInvalidIndex != argIndex )
	{
		if( _args[argIndex].ArgDesc.ArgType != CAT_Bool )
		{
			FxString errorMsg = "Argument Type Mismatch: ";
			errorMsg = FxString::Concat(errorMsg, argName);
			errorMsg = FxString::Concat(errorMsg, " expected FxBool!");
			FxVM::DisplayError(errorMsg);
		}
		else
		{
			if( _args[argIndex].Arg == "true" || _args[argIndex].Arg == "TRUE" )
			{
				arg = FxTrue;
			}
			else if( _args[argIndex].Arg == "false" || _args[argIndex].Arg == "FALSE" )
			{
				arg = FxFalse;
			}
			else
			{
				FxVM::DisplayError("Argument Type: FxBool expected true or false!");
				return FxFalse;
			}
			return FxTrue;
		}
	}
	return FxFalse;
}

FxSize FxCommandArgumentList::_getArgIndex( const FxString& argName ) const
{
	for( FxSize i = 0; i < _args.Length(); ++i )
	{
		if( _args[i].ArgDesc.ArgNameShort == argName ||
			_args[i].ArgDesc.ArgNameLong  == argName )
		{
			return i;
		}
	}
	return FxInvalidIndex;
}

//------------------------------------------------------------------------------
// FxCommandSyntax.
//------------------------------------------------------------------------------

FxCommandSyntax::FxCommandSyntax()
{
}

FxCommandSyntax::~FxCommandSyntax()
{
}

void FxCommandSyntax::AddArgDesc( const FxCommandArgumentDesc& argDesc )
{
	_syntax.PushBack(argDesc);
}

FxSize FxCommandSyntax::GetNumArgDescs( void ) const
{
	return _syntax.Length();
}

const FxCommandArgumentDesc& FxCommandSyntax::GetArgDesc( FxSize index ) const
{
	return _syntax[index];
}

//------------------------------------------------------------------------------
// FxCommand.
//------------------------------------------------------------------------------

#define kCurrentFxCommandVersion 0

FX_IMPLEMENT_CLASS(FxCommand, kCurrentFxCommandVersion, FxObject);

FxCommand::FxCommand()
	: _isUndoable(FxFalse)
	, _shouldFlushUndo(FxFalse)
	, _shouldEcho(FxTrue)
{
}

FxCommand::~FxCommand()
{
}

FxCommandSyntax FxCommand::CreateSyntax( void )
{
	return FxCommandSyntax();
}

FxCommandError FxCommand::Undo( void )
{
	return CE_Success;
}

FxCommandError FxCommand::Redo( void )
{
	return CE_Success;
}

FxBool FxCommand::IsUndoable( void ) const
{
	return _isUndoable;
}

FxBool FxCommand::ShouldFlushUndo( void ) const
{
	return _shouldFlushUndo;
}

FxBool FxCommand::ShouldEcho( void ) const
{
	return _shouldEcho;
}

void FxCommand::DisplayInfo( const FxString& msg )
{
	FxVM::DisplayInfo(msg);
}

void FxCommand::DisplayWarning( const FxString& msg )
{
	FxVM::DisplayWarning(msg);
}

void FxCommand::DisplayError( const FxString& msg )
{
	FxVM::DisplayError(msg);
}

const FxString& FxCommand::GetCommandString( void ) const
{
	return _commandString;
}

} // namespace Face

} // namespace OC3Ent
