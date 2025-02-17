//------------------------------------------------------------------------------
// The node group command.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxNodeGroupCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxNodeGroupCommand, 0, FxCommand);

FxNodeGroupCommand::FxNodeGroupCommand()
{
	_isUndoable = FxTrue;
	_selected   = FxFalse;
}

FxNodeGroupCommand::~FxNodeGroupCommand()
{
}

FxCommandSyntax FxNodeGroupCommand::CreateSyntax( void )
{
	FxCommandSyntax commandSyntax;
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-c", "-create", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-e", "-edit", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-r", "-remove", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-a", "-add", CAT_Flag));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-g", "-group", CAT_String));
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-nodes", CAT_StringArray));
	return commandSyntax;
}

FxCommandError FxNodeGroupCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-group", _requestedGroupName ) )
	{
		_previousGroupName = _requestedGroupName;
		FxSessionProxy::GetNodeGroup(_requestedGroupName, _previousGroupMembers);
	}
	else
	{
		FxVM::DisplayError("-group must be specified");
		return CE_Failure;
	}

	argList.GetArgument("-nodes", _requestedGroupMembers);

	if( argList.GetArgument("-create") )
	{
		_commandType = CreateGroup;
	}
	else if( argList.GetArgument("-edit") )
	{
		if( argList.GetArgument("-add") && _requestedGroupMembers.Length() )
		{
			// Remove any nodes that are in _requestedGroupMembers and are also in
			// _previousGroupMembers from _requestedGroupMembers.
			for( FxSize i = 0; i < _previousGroupMembers.Length(); ++i )
			{
				for( FxSize j = 0; j < _requestedGroupMembers.Length(); ++j )
				{
					if( _requestedGroupMembers[j] == _previousGroupMembers[i] )
					{
						_requestedGroupMembers.Remove(j);
					}
				}
			}
			_commandType = AddToGroup;
		}
		else if( argList.GetArgument("-remove") && _requestedGroupMembers.Length() )
		{
			_commandType = RemoveFromGroup;
		}
		else if( argList.GetArgument("-remove") && !_requestedGroupMembers.Length() )
		{
			_commandType = RemoveGroup;
			// Is the group selected?
			FxString selectedGroup;
			if( !FxSessionProxy::GetSelectedFaceGraphNodeGroup(selectedGroup) )
			{
				FxVM::DisplayError("could not get selected group!");
				return CE_Failure;
			}
			if( selectedGroup == _requestedGroupName )
			{
				_selected = FxTrue;
			}
		}
		else if( _requestedGroupMembers.Length() )
		{
			_commandType = SetGroup;
		}
		else
		{
			FxVM::DisplayError("No action specified.");
			return CE_Failure;
		}
	}
	else
	{
		// Nothing to do?
		FxVM::DisplayError("-edit or -create must be specified.");
		return CE_Failure;
	}

	// If we get here, we know what the user wanted to do.
	return Redo();
}

FxCommandError FxNodeGroupCommand::Undo( void )
{
	if( _commandType == RemoveGroup )
	{
		if( FxSessionProxy::CreateNodeGroup(_previousGroupName, _previousGroupMembers) )
		{
			if( _selected )
			{
				if( !FxSessionProxy::SetSelectedFaceGraphNodeGroup(_previousGroupName) )
				{
					FxVM::DisplayError("could not set node group selection!");
					return CE_Failure;
				}
			}
			return CE_Success;
		}
	}
	else if( _commandType == CreateGroup )
	{
		FxArray<FxString> emptyArray;
		if( FxSessionProxy::RemoveNodeGroup(_previousGroupName) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == AddToGroup )
	{
		if( FxSessionProxy::RemoveFromNodeGroup(_previousGroupName, _requestedGroupMembers) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == RemoveFromGroup )
	{
		if( FxSessionProxy::AddToNodeGroup(_previousGroupName, _requestedGroupMembers) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == SetGroup )
	{
		if( FxSessionProxy::SetNodeGroup(_previousGroupName, _previousGroupMembers) )
		{
			return CE_Success;
		}
	}
	return CE_Failure;
}

FxCommandError FxNodeGroupCommand::Redo( void )
{
	if( _commandType == RemoveGroup )
	{
		if( _selected )
		{
			if( !FxSessionProxy::SetSelectedFaceGraphNodeGroup("") )
			{
				FxVM::DisplayError("could not set node group selection!");
				return CE_Failure;
			}
		}
		FxArray<FxString> emptyArray;
		if( FxSessionProxy::RemoveNodeGroup(_requestedGroupName) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == CreateGroup )
	{
		if( FxSessionProxy::CreateNodeGroup(_requestedGroupName, _requestedGroupMembers) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == AddToGroup )
	{
		if( FxSessionProxy::AddToNodeGroup(_requestedGroupName, _requestedGroupMembers) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == RemoveFromGroup )
	{
		if( FxSessionProxy::RemoveFromNodeGroup(_requestedGroupName, _requestedGroupMembers) )
		{
			return CE_Success;
		}
	}
	else if( _commandType == SetGroup )
	{
		if( FxSessionProxy::SetNodeGroup(_requestedGroupName, _requestedGroupMembers) )
		{
			return CE_Success;
		}
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
