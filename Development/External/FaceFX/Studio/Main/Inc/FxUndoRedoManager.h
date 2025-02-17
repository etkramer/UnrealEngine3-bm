//------------------------------------------------------------------------------
// The FaceFx undo / redo manager.  This module is responsible for maintaining
// the undo / redo stacks and processing undo / redo commands.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxUndoRedoManager_H__
#define FxUndoRedoManager_H__

#include "FxPlatform.h"
#include "FxArray.h"
#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

class FxUndoRedoManager
{
public:
	// Execute the undo operation.
	static void Undo( FxBool noRedo = FxFalse );
	// Execute the redo operation.
	static void Redo( FxBool noUndo = FxFalse );

	// Returns FxTrue if there are commands available to undo.
	static FxBool CommandsToUndo( void );
	// Returns FxTrue if there are commands available to redo.
	static FxBool CommandsToRedo( void );

	// Adds a command to the undo stack.
	static void PushUndoableCommand( FxCommand* pCommand );
	// Flushes the undo / redo system.
	static void Flush( void );

	// Internal.
	static void SetFlushRedoStackOnExecute( FxBool flushRedoStack );
	static void DumpUndoStack( void );
	static void DumpRedoStack( void );

protected:
	// The undo stack.
	static FxArray<FxCommand*> _undoStack;
	// The redo stack.
	static FxArray<FxCommand*> _redoStack;
	// FxTrue if the redo stack should be flushed when a command is executed.
	static FxBool _flushRedoStack;
};

} // namespace Face

} // namespace OC3Ent

#endif
