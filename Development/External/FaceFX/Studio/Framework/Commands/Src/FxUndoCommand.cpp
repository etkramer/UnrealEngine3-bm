//------------------------------------------------------------------------------
// The undo command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxUndoCommand.h"
#include "FxUndoRedoManager.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxUndoCommand, 0, FxCommand);

FxUndoCommand::FxUndoCommand()
{
	_shouldEcho = FxFalse;
}

FxUndoCommand::~FxUndoCommand()
{
}

FxCommandSyntax FxUndoCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nr", "-noredo", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-d", "-debug", CAT_Flag));
	return newSyntax;
}

FxCommandError FxUndoCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-debug") )
	{
		FxUndoRedoManager::DumpUndoStack();
	}
	else
	{
		FxBool noRedo = FxFalse;
		if( argList.GetArgument("-noredo") )
		{
			noRedo = FxTrue;
		}
		FxUndoRedoManager::Undo(noRedo);
	}
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
