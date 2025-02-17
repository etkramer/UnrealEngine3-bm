//------------------------------------------------------------------------------
// The get command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxGetCommand.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxGetCommand, 0, FxCommand);

FxGetCommand::FxGetCommand()
{
	_shouldEcho = FxFalse;
}

FxGetCommand::~FxGetCommand()
{
}

FxCommandSyntax FxGetCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	return newSyntax;
}

FxCommandError FxGetCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString cvarName;
	if( !argList.GetArgument("-name", cvarName) )
	{
		FxVM::DisplayError("-name was not specified!");
		return CE_Failure;
	}

	FxConsoleVariable* pCvar = FxVM::FindConsoleVariable(cvarName);
	if( pCvar )
	{
		FxString msg = cvarName;
		msg = FxString::Concat(msg, " value: ");
		msg = FxString::Concat(msg, pCvar->GetString());
		msg = FxString::Concat(msg, " default: ");
		msg = FxString::Concat(msg, pCvar->GetDefaultValue());
		FxVM::DisplayInfo(msg, FxTrue);
		return CE_Success;
	}
	FxString msg = cvarName;
	msg = FxString::Concat(msg, " is undeclared!");
	FxVM::DisplayError(msg);
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
