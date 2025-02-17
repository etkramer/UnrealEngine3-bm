//------------------------------------------------------------------------------
// The save actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxSaveActorCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxSaveActorCommand, 0, FxCommand);

FxSaveActorCommand::FxSaveActorCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxSaveActorCommand::~FxSaveActorCommand()
{
}

FxCommandSyntax FxSaveActorCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	return newSyntax;
}

FxCommandError FxSaveActorCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString actorFilePath;
	if( argList.GetArgument("-file", actorFilePath) )
	{
		if( FxSessionProxy::SaveActor(actorFilePath) )
		{
			return CE_Success;
		}
	}
	else
	{
		FxVM::DisplayError("-file was not specified!");
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
