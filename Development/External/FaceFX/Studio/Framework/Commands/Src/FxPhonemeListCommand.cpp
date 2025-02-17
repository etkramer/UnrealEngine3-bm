//------------------------------------------------------------------------------
// Phoneme list commands.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxPhonemeListCommand.h"
#include "FxSessionProxy.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxPhonemeListCommand, 0, FxCommand);

// Constructor
FxPhonemeListCommand::FxPhonemeListCommand()
{
	_isUndoable = FxTrue;
}

// Destructor
FxPhonemeListCommand::~FxPhonemeListCommand()
{
}

// Sets up the argument syntax.
FxCommandSyntax FxPhonemeListCommand::CreateSyntax( void )
{
	FxCommandSyntax commandSyntax;
	commandSyntax.AddArgDesc(FxCommandArgumentDesc("-p", "-push", CAT_Flag));
	return commandSyntax;
}

// Execute the command.
FxCommandError FxPhonemeListCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument( "-push" ) )
	{
		if( FxSessionProxy::GetPhonWordList( &_redoState ) )
		{
			return Redo();
		}
		FxVM::DisplayError("Phoneme list is NULL");
		return CE_Failure;
	}
	else
	{
		FxVM::DisplayError("No argument specified.");
		return CE_Failure;
	}
}

// Undoes the command.
FxCommandError FxPhonemeListCommand::Undo( void )
{
	if( FxSessionProxy::GetPhonWordList( &_redoState ) && FxSessionProxy::SetPhonWordList( &_undoState ) )
	{
		return CE_Success;
	}
	return CE_Failure;
}

// Redoes the command.
FxCommandError FxPhonemeListCommand::Redo( void )
{
	if( FxSessionProxy::GetPhonWordList( &_undoState ) && FxSessionProxy::SetPhonWordList( &_redoState ) )
	{
		return CE_Success;
	}
	return CE_Failure;
}

} // namespace Face

} // namespace OC3Ent
