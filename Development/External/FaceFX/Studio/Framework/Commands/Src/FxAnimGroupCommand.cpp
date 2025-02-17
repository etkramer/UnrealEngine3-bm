//------------------------------------------------------------------------------
// The animation group command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxAnimGroupCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxAnimGroupCommand, 0, FxCommand);

FxAnimGroupCommand::FxAnimGroupCommand()
{
	_isUndoable = FxTrue;
	_create     = FxFalse;
	_remove		= FxFalse;
}

FxAnimGroupCommand::~FxAnimGroupCommand()
{
}

FxCommandSyntax FxAnimGroupCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-c", "-create", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-r", "-remove", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-g", "-group", CAT_String));
	return newSyntax;
}

FxCommandError FxAnimGroupCommand::Execute( const FxCommandArgumentList& argList )
{
	_create = argList.GetArgument("-create");
	_remove = argList.GetArgument("-remove");

	if( _create && _remove )
	{
		FxVM::DisplayError("-create and -remove both specified!");
		return CE_Failure;
	}
	else if( !_create && !_remove )
	{
		FxVM::DisplayError("-create or -remove must be specified!");
		return CE_Failure;
	}

	if( !argList.GetArgument("-group", _group) )
	{
		FxVM::DisplayError("-group was not specified!");
		return CE_Failure;
	}

	return Redo();
}

FxCommandError FxAnimGroupCommand::Undo( void )
{
	if( _create )
	{
		if( FxSessionProxy::RemoveAnimGroup(_group) )
		{
			return CE_Success;
		}
	}
	if( _remove )
	{
		if( FxSessionProxy::CreateAnimGroup(_group) )
		{
			return CE_Success;
		}
	}
	return CE_Failure;
}

FxCommandError FxAnimGroupCommand::Redo( void )
{
	if( _create )
	{
		if( FxSessionProxy::CreateAnimGroup(_group) )
		{
			return CE_Success;
		}
		else
		{
			FxVM::DisplayError("could not create group - make sure it is not a duplicate.");
		}
	}
	if( _remove )
	{
		if( FxSessionProxy::RemoveAnimGroup(_group) )
		{
			return CE_Success;
		}
		else
		{
			FxVM::DisplayError("could not remove group - make sure it is empty and is not a mounted group.");
		}
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
