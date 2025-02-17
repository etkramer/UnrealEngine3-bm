//------------------------------------------------------------------------------
// The key command.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxKeyCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxKeyCommand, 0, FxCommand);

FxKeyCommand::FxKeyCommand()
	: _commandMode( NONE )
{
	_isUndoable = FxTrue;
}

FxKeyCommand::~FxKeyCommand()
{
}

FxCommandSyntax FxKeyCommand::CreateSyntax( void )
{
	FxCommandSyntax commandSyntax;
	// Flags and parameters for key selection.
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-s", "-select", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-minT", "-minTime", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-maxT", "-maxTime", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-minV", "-minValue", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-maxV", "-maxValue", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-c", "-clear", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-tg", "-toggle", CAT_Flag));
	// Flags and parameters for key operations.
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-i", "-insert", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-r", "-removeSelection", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-x", "-transform", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-fg", "-fromgui", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-t", "-time", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-v", "-value", CAT_Real));
	// Flags and parameters for pivot key operations.
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-p", "-pivot", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-set", "-set", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-ci", "-curveIndex", CAT_Size));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-ki", "-keyIndex", CAT_Size));
	// Flags and parameters for key edits
	// Uses (-curveIndex and -keyIndex) or -pivot as the key identifier.
	// Uses -time and -value for the requested time and values.
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-e", "-edit", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-si", "-slopeIn", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-so", "-slopeOut", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-psi", "-prevSlopeIn", CAT_Real));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-pso", "-prevSlopeOut", CAT_Real));

	// Flags for key insertion
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-d", "-default", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-nc", "-noClearSelection", CAT_Flag));
	return commandSyntax;
}

FxCommandError FxKeyCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-select") )
	{
		if( FxSessionProxy::GetSelectedKeys(_cachedSelCurveIndices, _cachedSelKeyIndices) )
		{
			if( argList.GetArgument("-clear") )
			{
				_commandMode = CLEARSELECTION;
				return Redo();
			}

			if( argList.GetArgument("-curveIndex", _requestedCurveIndex) &&
				argList.GetArgument("-keyIndex", _requestedKeyIndex) )
			{
				_singleKeySelection = FxTrue;
				_commandMode = SELECTION;
				return Redo();
			}
			else
			{
				_singleKeySelection = FxFalse;
			}

			if( !argList.GetArgument("-minTime", _requestedMinTime) )
			{
				_requestedMinTime = FxInvalidValue;
			}
			if( !argList.GetArgument("-maxTime", _requestedMaxTime) )
			{
				_requestedMaxTime = FxInvalidValue;
			}
			if( !argList.GetArgument("-minValue", _requestedMinValue) )
			{
				_requestedMinValue = FxInvalidValue;
			}
			if( !argList.GetArgument("-maxValue", _requestedMaxValue) )
			{
				_requestedMaxValue = FxInvalidValue;
			}
			_requestedToggle = argList.GetArgument("-toggle");
			_commandMode = SELECTION;
			return Redo();
		}
	}
	else if( argList.GetArgument("-insert") )
	{
		if( argList.GetArgument("-default") )
		{
			FxSessionProxy::GetSelectedKeys(_cachedSelCurveIndices, _cachedSelKeyIndices);
			_commandMode = DEFAULTINSERTION;
		}
		else if( argList.GetArgument("-curveIndex", _requestedCurveIndex) )
		{
			_fromGUI = argList.GetArgument("-noclearselection");
			_commandMode = INSERTION;
		}
		else
		{
			FxVM::DisplayError("-curveIndex or -default must be specified with -insert.");
			return CE_Failure;
		}

		if( !argList.GetArgument("-time", _requestedKeyTime) )
		{
			_requestedKeyTime = FxInvalidValue;
		}
		if( !argList.GetArgument("-value", _requestedKeyValue) )
		{
			_requestedKeyTime = FxInvalidValue;
		}
		if( _requestedKeyTime == FxInvalidValue && _requestedKeyValue == FxInvalidValue )
		{
			FxVM::DisplayError("Both -time and -value must be specified.");
			return CE_Failure;
		}

		if( _commandMode != DEFAULTINSERTION )
		{
			if( !argList.GetArgument("-slopeIn", _requestedKeySlopeIn) )
			{
				_requestedKeySlopeIn = FxInvalidValue;
			}
			if( !argList.GetArgument("-slopeOut", _requestedKeySlopeOut) )
			{
				_requestedKeySlopeOut = FxInvalidValue;
			}
		}

		return Redo();
	}
	else if( argList.GetArgument("-removeSelection") )
	{
		if( FxSessionProxy::GetSelectedKeys(_cachedSelCurveIndices, _cachedSelKeyIndices) &&
			FxSessionProxy::GetSelectedKeyInfos(_cachedKeyTimes, _cachedKeyValues, _cachedKeyIncDerivs, _cachedKeyOutDerivs, _cachedKeyFlags) )
		{
			_commandMode = DELETION;
			return Redo();
		}
	}
	else if( argList.GetArgument("-transform") )
	{
		_commandMode = TRANSFORMATION;
		_fromGUI = argList.GetArgument("-fromgui");
		_requestedKeySlopeIn = FxInvalidValue;
		if( !argList.GetArgument("-time", _requestedTimeDelta) )
		{
			_requestedTimeDelta = FxInvalidValue;
		}
		if( !argList.GetArgument("-value", _requestedValueDelta) )
		{
			_requestedValueDelta = FxInvalidValue;
		}


		if( _requestedTimeDelta == FxInvalidValue && _requestedValueDelta == FxInvalidValue )
		{
			if( !argList.GetArgument("-slopeIn", _requestedKeySlopeIn) )
			{
				FxVM::DisplayError("-time and/or -value or -slopeIn must be specified with -transform.");
				return CE_Failure;
			}
		}

		if( _requestedKeySlopeIn != FxInvalidValue )
		{
			FxSessionProxy::GetSelectedKeys(_cachedSelCurveIndices, _cachedSelKeyIndices);
			FxSessionProxy::GetSelectedKeyInfos(_cachedKeyTimes, _cachedKeyValues, _cachedKeyIncDerivs, _cachedKeyOutDerivs, _cachedKeyFlags);
		}
		return Redo();
	}
	else if( argList.GetArgument("-pivot") )
	{
		if( argList.GetArgument("-set") &&
			argList.GetArgument("-curveIndex", _requestedCurveIndex) &&
			argList.GetArgument("-keyIndex", _requestedKeyIndex) )
		{
			FxSessionProxy::GetPivotKey( _cachedCurveIndex, _cachedKeyIndex );
			_commandMode = SETPIVOT;
			return Redo();
		}
		else
		{
			FxVM::DisplayError("-curveIndex and -keyIndex must be specified with -pivot.");
			return CE_Failure;
		}
	}
	else if( argList.GetArgument("-edit") )
	{
		// Check that we know what key to modify.
		if( argList.GetArgument("-curveIndex", _requestedCurveIndex) &&
			argList.GetArgument("-keyIndex", _requestedKeyIndex) )
		{
		}
		else if( argList.GetArgument("-pivot") )
		{
			FxSessionProxy::GetPivotKey(_requestedCurveIndex, _requestedKeyIndex);
		}
		else
		{
			FxVM::DisplayError("Must specify -curveIndex and -keyIndex or -pivot with -edit.");
			return CE_Failure;
		}

		// Cache the current state of the key.
		if( !FxSessionProxy::GetKeyInfo(_requestedCurveIndex, _requestedKeyIndex,
			_cachedKeyTime, _cachedKeyValue, _cachedKeySlopeIn, _cachedKeySlopeOut) )
		{
			FxVM::DisplayError("The requested curve or key does not exist.");
			return CE_Failure;
		}

		FxReal temp;
		if( argList.GetArgument("-prevSlopeIn", temp) )
		{
			_cachedKeySlopeIn = temp;
		}
		if( argList.GetArgument("-prevSlopeOut", temp) )
		{
			_cachedKeySlopeOut = temp;
		}

		// Load the parameters
		if( !argList.GetArgument("-time", _requestedKeyTime) )
		{
			_requestedKeyTime = _cachedKeyTime;
		}
		if( !argList.GetArgument("-value", _requestedKeyValue) )
		{
			_requestedKeyValue = _cachedKeyValue;
		}
		if( !argList.GetArgument("-slopeIn", _requestedKeySlopeIn) )
		{
			_requestedKeySlopeIn = _cachedKeySlopeIn;
		}
		if( !argList.GetArgument("-slopeOut", _requestedKeySlopeOut) )
		{
			_requestedKeySlopeOut = _cachedKeySlopeOut;
		}

		// Make sure at least one parameter was specified.
		if( _requestedKeyTime == FxInvalidValue && _requestedKeyValue == FxInvalidValue &&
			_requestedKeySlopeIn == FxInvalidValue && _requestedKeySlopeOut == FxInvalidValue )
		{
			FxVM::DisplayError("Must specify one of: -time, -value, -slopeIn, -slopeOut.");
			return CE_Failure;
		}

		// We should be good to go here.
		_commandMode = EDIT;
		return Redo();
	}
	return CE_Failure;
}

FxCommandError FxKeyCommand::Undo( void )
{
	if( _commandMode == CLEARSELECTION || _commandMode == SELECTION )
	{
		if( FxSessionProxy::SetSelectedKeys(_cachedSelCurveIndices, _cachedSelKeyIndices) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == TRANSFORMATION )
	{
		if( _requestedKeySlopeIn != FxInvalidValue )
		{
			FxSessionProxy::RemoveKeySelection();
			if( FxSessionProxy::InsertKeys(_cachedSelCurveIndices, _cachedKeyTimes, _cachedKeyValues,
				_cachedKeyIncDerivs, _cachedKeyOutDerivs, _cachedKeyFlags) )
			{
				return CE_Success;
			}
		}
		else
		{
			FxReal undoTimeDelta = _requestedTimeDelta == FxInvalidValue ? FxInvalidValue : -1 * _requestedTimeDelta;
			FxReal undoValueDelta = _requestedValueDelta == FxInvalidValue ? FxInvalidValue : -1 * _requestedValueDelta;;
			if( FxSessionProxy::TransformSelectedKeys(undoTimeDelta, undoValueDelta) )
			{
				return CE_Success;
			}
		}
	}
	else if( _commandMode == SETPIVOT )
	{
		if( FxSessionProxy::SetPivotKey(_cachedCurveIndex, _cachedKeyIndex) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == DELETION )
	{
		if( FxSessionProxy::InsertKeys(_cachedSelCurveIndices, _cachedKeyTimes, _cachedKeyValues,
			_cachedKeyIncDerivs, _cachedKeyOutDerivs, _cachedKeyFlags) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == EDIT )
	{
		if( FxSessionProxy::EditKeyInfo(_requestedCurveIndex, _requestedKeyIndex,
			_cachedKeyTime, _cachedKeyValue, _cachedKeySlopeIn, _cachedKeySlopeOut) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == INSERTION )
	{
		if( FxSessionProxy::RemoveKey(_requestedCurveIndex, _cachedKeyIndex) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == DEFAULTINSERTION )
	{
		if( FxSessionProxy::RemoveKeySelection() )
		{
			FxSessionProxy::SetSelectedKeys(_cachedSelCurveIndices, _cachedSelKeyIndices);
			return CE_Success;
		}
	}
	return CE_Failure;
}

FxCommandError FxKeyCommand::Redo( void )
{
	if( _commandMode == CLEARSELECTION )
	{
		if( FxSessionProxy::ClearSelectedKeys() )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == SELECTION )
	{
		if( _singleKeySelection )
		{
			FxArray<FxSize> curveIndices;
			FxArray<FxSize> keyIncices;
			curveIndices.PushBack(_requestedCurveIndex);
			keyIncices.PushBack(_requestedKeyIndex);
			if( FxSessionProxy::SetSelectedKeys(curveIndices, keyIncices) )
			{
				return CE_Success;
			}
		}
		else
		{
			if( FxSessionProxy::ApplyKeySelection(_requestedMinTime, _requestedMaxTime, _requestedMinValue, _requestedMaxValue, _requestedToggle) )
			{
				return CE_Success;
			}
		}
	}
	else if( _commandMode == TRANSFORMATION )
	{	
		if( _fromGUI )
		{
			_fromGUI = FxFalse;
			return CE_Success;
		
		}
		if( _requestedKeySlopeIn != FxInvalidValue )
		{
			if( FxSessionProxy::SetSelectedKeysSlopes(_requestedKeySlopeIn) )
			{
				return CE_Success;
			}
		}
		else if( FxSessionProxy::TransformSelectedKeys(_requestedTimeDelta, _requestedValueDelta) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == SETPIVOT )
	{
		if( FxSessionProxy::SetPivotKey(_requestedCurveIndex, _requestedKeyIndex) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == DELETION )
	{
		if( FxSessionProxy::RemoveKeySelection() )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == EDIT )
	{
		if( FxSessionProxy::EditKeyInfo(_requestedCurveIndex, _requestedKeyIndex,
				_requestedKeyTime, _requestedKeyValue, _requestedKeySlopeIn, _requestedKeySlopeOut) )
		{
			return CE_Success;
		}
	}
	else if( _commandMode == INSERTION )
	{
		if( _requestedKeySlopeIn == FxInvalidValue || _requestedKeySlopeOut == FxInvalidValue )
		{
			if( FxSessionProxy::InsertKey(_requestedCurveIndex, _requestedKeyTime, 
				_requestedKeyValue, 0.0f, 0.0f, _cachedKeyIndex, _fromGUI, FxTrue) )
			{
				return CE_Success;
			}
		}
		else
		{
			if( FxSessionProxy::InsertKey(_requestedCurveIndex, _requestedKeyTime, 
				_requestedKeyValue, _requestedKeySlopeIn, _requestedKeySlopeOut, _cachedKeyIndex, _fromGUI) )
			{
				return CE_Success;
			}
		}
	}
	else if( _commandMode == DEFAULTINSERTION )
	{
		if( FxSessionProxy::DefaultInsertKey(_requestedKeyTime, _requestedKeyValue) )
		{
			return CE_Success;
		}
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
