//------------------------------------------------------------------------------
// The rename actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxRenameActorCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxRenameActorCommand, 0, FxCommand);

FxRenameActorCommand::FxRenameActorCommand()
{
	_isUndoable = FxTrue;
}

FxRenameActorCommand::~FxRenameActorCommand()
{
}

FxCommandSyntax FxRenameActorCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	return newSyntax;
}

FxCommandError FxRenameActorCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-name", _requestedName) )
	{
		if( FxSessionProxy::GetActorName(_previousName) )
		{
			if( _previousName != _requestedName )
			{
				return Redo();
			}
			else // _previousName == _requestedName
			{
				// Specifically allow this so that simply hitting "OK" on the
				// Rename Actor... dialog (which also pops up when you simply
				// do File->New Actor...) does not produce a failed command
				// execution error, though it is not undoable to avoid confusion.
				_isUndoable = FxFalse;
				return CE_Success;
			}
		}
	}
	else
	{
		FxVM::DisplayError("-name was not specified!");
	}
	return CE_Failure;
}

FxCommandError FxRenameActorCommand::Undo( void )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		pActor->SetName(_previousName.GetData());
		if( !FxSessionProxy::ActorDataChanged() )
		{
			FxVM::DisplayError("internal session error!");
			return CE_Failure;
		}
		return CE_Success;
	}
	else
	{
		FxVM::DisplayError("null actor!");
	}
	return CE_Failure;
}

FxCommandError FxRenameActorCommand::Redo( void )
{
	FxActor* pActor = NULL;
	FxSessionProxy::GetActor(&pActor);
	if( pActor )
	{
		pActor->SetName(_requestedName.GetData());
		if( !FxSessionProxy::ActorDataChanged() )
		{
			FxVM::DisplayError("internal session error!");
			return CE_Failure;
		}
		return CE_Success;
	}
	else
	{
		FxVM::DisplayError("null actor!");
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
