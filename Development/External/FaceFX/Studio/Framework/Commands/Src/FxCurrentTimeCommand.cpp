//------------------------------------------------------------------------------
// The current time command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxCurrentTimeCommand.h"
#include "FxVM.h"
#include "FxSessionProxy.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxCurrentTimeCommand, 0, FxCommand);

FxCurrentTimeCommand::FxCurrentTimeCommand()
{
	_isUndoable    = FxTrue;
	_previousTime  = 0.0f;
	_requestedTime = 0.0f;
	_requestedFrame = 0;
	_frameMode = FxFalse;
}

FxCurrentTimeCommand::~FxCurrentTimeCommand()
{
}

FxCommandSyntax FxCurrentTimeCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-new", CAT_Real));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-f", "-frame", CAT_Int32));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-p", "-prev", CAT_Real));
	return newSyntax;
}

FxCommandError FxCurrentTimeCommand::Execute( const FxCommandArgumentList& argList )
{
	if( !argList.GetArgument("-prev", _previousTime) )
	{
		FxSessionProxy::GetSessionTime(_previousTime);
	}
	if( argList.GetArgument("-new", _requestedTime) )
	{
		return Redo();
	}
	else if( argList.GetArgument("-frame", _requestedFrame) )
	{
		_frameMode = FxTrue;
		return Redo();
	}
	return CE_Failure;
}

FxCommandError FxCurrentTimeCommand::Undo( void )
{
	if( FxSessionProxy::SetSessionTime(_previousTime) )
	{
		return CE_Success;
	}
	return CE_Failure;
}

FxCommandError FxCurrentTimeCommand::Redo( void )
{
	if( _frameMode )
	{
		if( FxSessionProxy::SetSessionFrame(_requestedFrame) ) return CE_Success;
	}
	else if( FxSessionProxy::SetSessionTime(_requestedTime) )
	{
		return CE_Success;
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
