//------------------------------------------------------------------------------
// The exit command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxExitCommand.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxExitCommand, 0, FxCommand);

FxExitCommand::FxExitCommand()
{
}

FxExitCommand::~FxExitCommand()
{
}

FxCommandError FxExitCommand::Execute( const FxCommandArgumentList& /*argList*/ )
{
	FxSessionProxy::ExitApplication();
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
