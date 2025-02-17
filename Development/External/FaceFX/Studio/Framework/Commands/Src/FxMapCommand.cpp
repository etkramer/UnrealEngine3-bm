//------------------------------------------------------------------------------
// The commands governing the phoneme to track map.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxMapCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxMapCommand, 0, FxCommand);

FxMapCommand::FxMapCommand()
{
	_isUndoable = FxTrue;
}

FxMapCommand::~FxMapCommand()
{
}

FxCommandSyntax FxMapCommand::CreateSyntax( void )
{
	FxCommandSyntax commandSyntax;
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-c", "-create", CAT_Flag) );
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-r", "-remove", CAT_Flag) );
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-s", "-set", CAT_Flag) );
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-n", "-norefresh", CAT_Flag) );
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-t", "-track", CAT_String) );
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-p", "-phoneme", CAT_String) );
	commandSyntax.AddArgDesc( FxCommandArgumentDesc("-v", "-value", CAT_Real) );
	return commandSyntax;
}

FxCommandError FxMapCommand::Execute( const FxCommandArgumentList& argList )
{
	_noRefresh = argList.GetArgument("-norefresh");

	if( argList.GetArgument("-create") )
	{
		if( argList.GetArgument("-track", _requestedTrackName) )
		{
			_commandMode = CreateTrack;
			return Redo();
		}
		else
		{
			FxVM::DisplayError("Only -track may be specified with -create");
			return CE_Failure;
		}
	}
	else if( argList.GetArgument("-remove") )
	{
		if( argList.GetArgument("-phoneme", _requestedPhonemeString) )
		{
			_commandMode = RemovePhoneme;
			if( FxSessionProxy::GetPhonemeMapping( _requestedPhonemeString, _currentNames, _currentValues ) )
			{
				return Redo();
			}
			else
			{
				return CE_Failure;
			}
		}
		else if( argList.GetArgument("-track", _requestedTrackName) )
		{
			_commandMode = RemoveTrack;
			if( FxSessionProxy::GetTrackMapping( _requestedTrackName, _currentNames, _currentValues ) )
			{
				return Redo();
			}
			else
			{
				return CE_Failure;
			}
		}
		else
		{
			FxVM::DisplayError("Only -phoneme or -track may be specified with -remove");
			return CE_Failure;
		}
	}
	else if( argList.GetArgument("-set") )
	{
		if( argList.GetArgument("-phoneme", _requestedPhonemeString) &&
			argList.GetArgument("-track", _requestedTrackName) &&
			argList.GetArgument("-value", _requestedValue) )
		{
			_commandMode = MapPhonemeToTrack;
			if( FxSessionProxy::GetMappingEntry( _requestedPhonemeString, _requestedTrackName, _currentValue ) )
			{
				return Redo();
			}
			else
			{
				return CE_Failure;
			}
		}
		else
		{
			FxVM::DisplayError("Must specify -phoneme and -track and -value with -set");
			return CE_Failure;
		}
	}
	return CE_Failure;
}

FxCommandError FxMapCommand::Undo( void )
{
	switch( _commandMode )
	{
	case CreateTrack:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::RemoveTrackFromMapping(_requestedTrackName);
			if( _noRefresh )
			{		
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	case RemoveTrack:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::AddTrackToMapping(_requestedTrackName);
			// Recreate the values that were in the track.
			for( FxSize i = 0; i < _currentNames.Length(); ++i )
			{
				FxSessionProxy::SetMappingEntry(_currentNames[i], _requestedTrackName, _currentValues[i]);
			}
			if( _noRefresh )
			{		
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	case MapPhonemeToTrack:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::SetMappingEntry(_requestedPhonemeString, _requestedTrackName, _currentValue);
			if( _noRefresh )
			{		
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	case RemovePhoneme:
		{
			FxSessionProxy::BeginMappingBatch();
			// Recreate the values that were in the phoneme.
			for( FxSize i = 0; i < _currentNames.Length(); ++i )
			{
				FxSessionProxy::SetMappingEntry(_requestedPhonemeString, _currentNames[i], _currentValues[i]);
			}
			if( _noRefresh )
			{		
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return CE_Success;
		}
		break;
	default:
		FxVM::DisplayError("Unknown command mode");
		return CE_Failure;
	};
}

FxCommandError FxMapCommand::Redo( void )
{
	switch( _commandMode )
	{
	case CreateTrack:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::AddTrackToMapping(_requestedTrackName);
			if( _noRefresh )
			{		
				_noRefresh = FxFalse;
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	case RemoveTrack:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::RemoveTrackFromMapping(_requestedTrackName);
			if( _noRefresh )
			{		
				_noRefresh = FxFalse;
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	case MapPhonemeToTrack:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::SetMappingEntry( _requestedPhonemeString, _requestedTrackName, _requestedValue );
			if( _noRefresh )
			{		
				_noRefresh = FxFalse;
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	case RemovePhoneme:
		{
			FxSessionProxy::BeginMappingBatch();
			FxBool retval = FxSessionProxy::ClearPhonemeMapping( _requestedPhonemeString );
			if( _noRefresh )
			{		
				_noRefresh = FxFalse;
				FxSessionProxy::EndMappingBatchNoRefresh();
			}
			else
			{
				FxSessionProxy::EndMappingBatch();
			}
			return retval ? CE_Success : CE_Failure;
		}
		break;
	default:
		FxVM::DisplayError("Unknown command mode");
		return CE_Failure;
	};
}

} // namespace Face

} // namespace OC3Ent
