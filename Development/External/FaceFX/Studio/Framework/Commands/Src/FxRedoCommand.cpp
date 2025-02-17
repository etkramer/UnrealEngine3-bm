//------------------------------------------------------------------------------
// The redo command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxRedoCommand.h"
#include "FxUndoRedoManager.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxRedoCommand, 0, FxCommand);

FxRedoCommand::FxRedoCommand()
{
	_shouldEcho = FxFalse;
}

FxRedoCommand::~FxRedoCommand()
{
}

FxCommandSyntax FxRedoCommand::CreateSyntax( void )
{
	FxCommandSyntax newSyntax;
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-nu", "-noundo", CAT_Flag));
	newSyntax.AddArgDesc(FxCommandArgumentDesc("-d", "-debug", CAT_Flag));
	return newSyntax;
}

FxCommandError FxRedoCommand::Execute( const FxCommandArgumentList& argList )
{
	if( argList.GetArgument("-debug") )
	{
		FxUndoRedoManager::DumpRedoStack();
	}
	else
	{
		FxBool noUndo = FxFalse;
		if( argList.GetArgument("-noundo") )
		{
			noUndo = FxTrue;
		}
		FxUndoRedoManager::Redo(noUndo);
	}
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
