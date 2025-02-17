//------------------------------------------------------------------------------
// The print commands.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxPrintCommands.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxPrintCommand, 0, FxCommand);

FxPrintCommand::FxPrintCommand()
{
	_shouldEcho = FxFalse;
}

FxPrintCommand::~FxPrintCommand()
{
}

FxCommandSyntax FxPrintCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-m", "-message", CAT_String));
	return newSyntax;
}

FxCommandError FxPrintCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString message;
	if( !argList.GetArgument("-message", message) )
	{
		FxVM::DisplayError("-message was not specified!");
		return CE_Failure;
	}
	FxVM::DisplayInfo(message);
	return CE_Success;
}

FX_IMPLEMENT_CLASS(FxWarnCommand, 0, FxCommand);

FxWarnCommand::FxWarnCommand()
{
	_shouldEcho = FxFalse;
}

FxWarnCommand::~FxWarnCommand()
{
}

FxCommandSyntax FxWarnCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-m", "-message", CAT_String));
	return newSyntax;
}

FxCommandError FxWarnCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString message;
	if( !argList.GetArgument("-message", message) )
	{
		FxVM::DisplayError("-message was not specified!");
		return CE_Failure;
	}
	FxVM::DisplayWarning(message);
	return CE_Success;
}

FX_IMPLEMENT_CLASS(FxErrorCommand, 0, FxCommand);

FxErrorCommand::FxErrorCommand()
{
	_shouldEcho = FxFalse;
}

FxErrorCommand::~FxErrorCommand()
{
}

FxCommandSyntax FxErrorCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-m", "-message", CAT_String));
	return newSyntax;
}

FxCommandError FxErrorCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString message;
	if( !argList.GetArgument("-message", message) )
	{
		FxVM::DisplayError("-message was not specified!");
		return CE_Failure;
	}
	FxVM::DisplayError(message);
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
