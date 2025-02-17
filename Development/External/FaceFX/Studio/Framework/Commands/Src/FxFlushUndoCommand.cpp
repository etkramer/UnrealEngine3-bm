//------------------------------------------------------------------------------
// This command flushes the undo buffer.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxFlushUndoCommand.h"
#include "FxUndoRedoManager.h"

namespace OC3Ent
{

namespace Face
{

FX_IMPLEMENT_CLASS(FxFlushUndoCommand, 0, FxCommand);

FxFlushUndoCommand::FxFlushUndoCommand()
{
}

FxFlushUndoCommand::~FxFlushUndoCommand()
{
}

FxCommandError FxFlushUndoCommand::Execute( const FxCommandArgumentList& /*argList*/ )
{
	FxUndoRedoManager::Flush();
	return CE_Success;
}

} // namespace Face

} // namespace OC3Ent
