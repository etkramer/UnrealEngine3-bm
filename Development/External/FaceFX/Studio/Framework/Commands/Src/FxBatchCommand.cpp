//------------------------------------------------------------------------------
// The batch command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxBatchCommand.h"
#include "FxVM.h"
#include "FxUndoRedoManager.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxBeginBatchCommand, 0, FxCommand);

FxBeginBatchCommand::FxBeginBatchCommand()
{
	_shouldEcho = FxFalse;
}

FxBeginBatchCommand::~FxBeginBatchCommand()
{
}

FxCommandError FxBeginBatchCommand::Execute( const FxCommandArgumentList& /*argList*/ )
{
	if( !FxVM::BeginBatch() )
	{
		FxVM::DisplayError("could not begin batch!");
		return CE_Failure;
	}
	return CE_Success;
}

FX_IMPLEMENT_CLASS(FxExecBatchCommand, 0, FxCommand);

FxExecBatchCommand::FxExecBatchCommand()
{
	_shouldEcho = FxFalse;
}

FxExecBatchCommand::~FxExecBatchCommand()
{
}

FxCommandSyntax FxExecBatchCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-an", "-addednodes", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-rn", "-removednodes", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-en", "-editednodes", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ac", "-addedcurves", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-rc", "-removedcurves", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-cm", "-changedmapping", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ec", "-editedcurves", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ca", "-changedanimation", CAT_Flag));
	return newSyntax;
}

FxCommandError FxExecBatchCommand::Execute( const FxCommandArgumentList& argList )
{
	FxArray<FxString> flags;
	if( argList.GetArgument("-addednodes") )
	{
		flags.PushBack("-addednodes");
	}
	if( argList.GetArgument("-removednodes") )
	{
		flags.PushBack("-removednodes");
	}
	if( argList.GetArgument("-editednodes") )
	{
		flags.PushBack("-editednodes");
	}
	if( argList.GetArgument("-addedcurves") )
	{
		flags.PushBack("-addedcurves");
	}
	if( argList.GetArgument("-removedcurves") )
	{
		flags.PushBack("-removedcurves");
	}
	if( argList.GetArgument("-changedmapping") )
	{
		flags.PushBack("-changedmapping");
	}
	if( argList.GetArgument("-editedcurves") )
	{
		flags.PushBack("-editedcurves");
	}
	if( !FxVM::ExecBatch(flags) )
	{
		FxVM::DisplayError("could not execute batch!");
		return CE_Failure;
	}
	return CE_Success;
}

FX_IMPLEMENT_CLASS(FxBatchCommand, 0, FxCommand);

FxBatchCommand::FxBatchCommand()
{
	_isUndoable = FxFalse;
	_commandString = "execBatch;";
	_allCommandsWereUndoable = FxTrue;
	_undoCalled = FxFalse;
}

FxBatchCommand::~FxBatchCommand()
{
}

FxCommandSyntax FxBatchCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-an", "-addednodes", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-rn", "-removednodes", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-en", "-editednodes", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ac", "-addedcurves", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-rc", "-removedcurves", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-cm", "-changedmapping", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-ec", "-editedcurves", CAT_Flag));
	return newSyntax;
}

FxCommandError FxBatchCommand::Execute( const FxCommandArgumentList& argList )
{
	_addedNodes = argList.GetArgument("-addednodes");
	_removedNodes = argList.GetArgument("-removednodes");
	_editedNodes = argList.GetArgument("-editednodes");
	_addedCurves = argList.GetArgument("-addedcurves");
	_removedCurves = argList.GetArgument("-removedcurves");
	_mappingChanged = argList.GetArgument("-changedmapping");
	_editedCurves = argList.GetArgument("-editedcurves");

	return Redo();
}

//@todo Fix this horrible nasty code for the no actor changed messages as well
//      as mapping changed messages.
FxCommandError FxBatchCommand::Undo( void )
{
	FxVM::SetEchoCommands(FxFalse);
	if( _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves  )
	{
		FxSessionProxy::SetNoActorChangedMessages(FxTrue);
	}
	if( _editedCurves || _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves  )
	{
		FxSessionProxy::SetNoRefreshMessages(FxTrue);
	}
	if( _mappingChanged )
	{
		FxSessionProxy::BeginMappingBatch();
	}
	if( _allCommandsWereUndoable )
	{
		for( FxSize i = 0; i < _batchedCommands.Length(); ++i )
		{
			if( CE_Success != FxVM::ExecuteCommand("undo -noredo") )
			{
				FxVM::SetEchoCommands(FxTrue);
				FxSessionProxy::SetNoActorChangedMessages(FxFalse);
				FxSessionProxy::SetNoRefreshMessages(FxFalse);
				if( _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves )
				{
					FxActorDataChangedFlag flags = ADCF_All;
					if( (_addedCurves || _removedCurves) && !(_addedNodes || _removedNodes || _editedNodes) )
					{
						// If we only changed curves, we don't need to recompile the face graph.
						flags = ADCF_CurvesChanged;
					}
					FxSessionProxy::ActorDataChanged(flags);
				}
				if( _editedCurves || _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves )
				{
					FxSessionProxy::RefreshControls();
				}
				if( _mappingChanged )
				{
					FxSessionProxy::EndMappingBatch();
				}
				return CE_Failure;
			}
		}
		_undoCalled = FxTrue;
	}
	FxVM::SetEchoCommands(FxTrue);
	FxSessionProxy::SetNoActorChangedMessages(FxFalse);
	FxSessionProxy::SetNoRefreshMessages(FxFalse);
	if( _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves )
	{
		FxActorDataChangedFlag flags = ADCF_All;
		if( (_addedCurves || _removedCurves) && !(_addedNodes || _removedNodes || _editedNodes) )
		{
			// If we only changed curves, we don't need to recompile the face graph.
			flags = ADCF_CurvesChanged;
		}
		FxSessionProxy::ActorDataChanged(flags);
	}
	if( _editedCurves || _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves  )
	{
		FxSessionProxy::RefreshControls();
	}
	if( _mappingChanged )
	{
		FxSessionProxy::EndMappingBatch();
	}
	return CE_Success;
}

FxCommandError FxBatchCommand::Redo( void )
{
	if( _undoCalled )
	{
		FxUndoRedoManager::SetFlushRedoStackOnExecute(FxFalse);
	}
	FxVM::SetEchoCommands(FxFalse);
	if( _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves )
	{
		FxSessionProxy::SetNoActorChangedMessages(FxTrue);
	}
	if( _editedCurves || _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves  )
	{
		FxSessionProxy::SetNoRefreshMessages(FxTrue);
	}
	if( _mappingChanged )
	{
		FxSessionProxy::BeginMappingBatch();
	}
	for( FxSize i = 0; i < _batchedCommands.Length(); ++i )
	{
		if( CE_Success != FxVM::ExecuteCommand(_batchedCommands[i]) )
		{
			FxVM::SetEchoCommands(FxTrue);
			FxSessionProxy::SetNoActorChangedMessages(FxFalse);
			FxSessionProxy::SetNoRefreshMessages(FxFalse);
			if( _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves )
			{
				FxActorDataChangedFlag flags = ADCF_All;
				if( (_addedCurves || _removedCurves) && !(_addedNodes || _removedNodes || _editedNodes) )
				{
					// If we only changed curves, we don't need to recompile the face graph.
					flags = ADCF_CurvesChanged;
				}
				FxSessionProxy::ActorDataChanged(flags);
			}
			if( _editedCurves || _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves  )
			{
				FxSessionProxy::RefreshControls();
			}
			if( _mappingChanged )
			{
				FxSessionProxy::EndMappingBatch();
			}
			return CE_Failure;
		}
		if( !FxVM::LastCommandWasUndoable() )
		{
			_allCommandsWereUndoable = FxFalse;
		}
	}
	if( _allCommandsWereUndoable )
	{
		_isUndoable = FxTrue;
	}
	else
	{
		FxVM::DisplayWarning("One or more batched commands are not undoable, therefore undo is disabled for the entire batch.");
	}
	FxVM::SetEchoCommands(FxTrue);
	FxSessionProxy::SetNoActorChangedMessages(FxFalse);
	FxSessionProxy::SetNoRefreshMessages(FxFalse);
	if( _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves )
	{
		FxActorDataChangedFlag flags = ADCF_All;
		if( (_addedCurves || _removedCurves) && !(_addedNodes || _removedNodes || _editedNodes) )
		{
			// If we only changed curves, we don't need to recompile the face graph.
			flags = ADCF_CurvesChanged;
		}
		FxSessionProxy::ActorDataChanged(flags);
	}
	if( _editedCurves || _addedNodes || _removedNodes || _editedNodes || _addedCurves || _removedCurves  )
	{
		FxSessionProxy::RefreshControls();
	}
	if( _mappingChanged )
	{
		FxSessionProxy::EndMappingBatch();
	}
	FxUndoRedoManager::SetFlushRedoStackOnExecute(FxTrue);
	return CE_Success;
}

void FxBatchCommand::AddToBatch( const FxString& command )
{
	_batchedCommands.PushBack(command);
}

} // namespace Face

} // namespace OC3Ent
