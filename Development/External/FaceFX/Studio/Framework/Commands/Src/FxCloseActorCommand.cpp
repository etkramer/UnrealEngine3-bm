//------------------------------------------------------------------------------
// The close actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCloseActorCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxCloseActorCommand, 0, FxCommand);

FxCloseActorCommand::FxCloseActorCommand()
{
	_shouldFlushUndo = FxTrue;
}

FxCloseActorCommand::~FxCloseActorCommand()
{
}

FxCommandError FxCloseActorCommand::Execute( const FxCommandArgumentList& /*argList*/ )
{
	if( FxSessionProxy::CloseActor() )
	{
		return CE_Success;
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
