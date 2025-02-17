//------------------------------------------------------------------------------
// The new actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxNewActorCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxNewActorCommand, 0, FxCommand);

FxNewActorCommand::FxNewActorCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxNewActorCommand::~FxNewActorCommand()
{
}

FxCommandSyntax FxNewActorCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	return newSyntax;
}

FxCommandError FxNewActorCommand::Execute( const FxCommandArgumentList& argList )
{
	if( FxSessionProxy::CreateActor() )
	{
		FxString actorName;
		if( argList.GetArgument("-name", actorName) )
		{
			FxString renameCommand = "renameActor -name \"";
			renameCommand = FxString::Concat(renameCommand, actorName);
			renameCommand = FxString::Concat(renameCommand, "\";");
			return FxVM::ExecuteCommand(renameCommand);
		}
		return CE_Success;
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
