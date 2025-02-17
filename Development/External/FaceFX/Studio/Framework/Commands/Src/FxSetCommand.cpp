//------------------------------------------------------------------------------
// The set command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxSetCommand.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxSetCommand, 0, FxCommand);

FxSetCommand::FxSetCommand()
{
	_shouldEcho = FxFalse;
}

FxSetCommand::~FxSetCommand()
{
}

FxCommandSyntax FxSetCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-n", "-name", CAT_String));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-v", "-value", CAT_String));
	return newSyntax;
}

FxCommandError FxSetCommand::Execute( const FxCommandArgumentList& argList )
{
	FxString cvarName;
	if( !argList.GetArgument("-name", cvarName) )
	{
		FxVM::DisplayError("-name was not specified!");
		return CE_Failure;
	}
	FxString cvarValue;
	if( !argList.GetArgument("-value", cvarValue) )
	{
		FxVM::DisplayError("-value was not specified!");
		return CE_Failure;
	}

	FxConsoleVariable* pCvar = FxVM::FindConsoleVariable(cvarName);
	if( pCvar )
	{
		if( pCvar->GetFlags() & FX_CONSOLE_VARIABLE_READONLY )
		{
			FxString msg = cvarName;
			msg = FxString::Concat(msg, " is declared as read-only!");
			FxVM::DisplayError(msg);
			return CE_Failure;
		}
		else
		{
			pCvar->SetString(cvarValue);
			if( pCvar->GetOnChangeCallback() )
			{
				pCvar->GetOnChangeCallback()(*pCvar);
			}
		}
	}
	else
	{
        FxVM::RegisterConsoleVariable(cvarName, cvarValue);
	}
	return CE_Success;	
}

} // namespace Face

} // namespace OC3Ent
