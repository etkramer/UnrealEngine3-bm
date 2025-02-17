//------------------------------------------------------------------------------
// The load actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxLoadActorCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxLoadActorCommand, 0, FxCommand);

FxLoadActorCommand::FxLoadActorCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxLoadActorCommand::~FxLoadActorCommand()
{
}

FxCommandSyntax FxLoadActorCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-file", CAT_String));
	return newSyntax;
}

FxCommandError FxLoadActorCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString actorFilePath;
	if( argList.GetArgument("-file", actorFilePath) )
	{
		if( FxSessionProxy::LoadActor(actorFilePath) )
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
