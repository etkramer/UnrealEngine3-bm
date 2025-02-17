//------------------------------------------------------------------------------
// The FaceFx undo / redo manager.  This module is responsible for maintaining
// the undo / redo stacks and processing undo / redo commands.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#include "stdwx.h"

#include "FxUndoRedoManager.h"
#include "FxVM.h"

namespace OC3Ent
{

namespace Face
{

FxArray<FxCommand*> FxUndoRedoManager::_undoStack;
FxArray<FxCommand*> FxUndoRedoManager::_redoStack;
FxBool FxUndoRedoManager::_flushRedoStack = FxTrue;

void FxUndoRedoManager::Undo( FxBool noRedo )
{
	if( _undoStack.Length() > 0 )
	{
		FxCommand* pCommand = _undoStack.Back();
		_undoStack.PopBack();
		pCommand->Undo();
		FxString undoString = "undo ";
		undoString = FxString::Concat(undoString, pCommand->GetCommandString());
		FxVM::DisplayEcho(undoString);
		if( !noRedo )
		{
			_redoStack.PushBack(pCommand);
		}
		else
		{
			delete pCommand;
			pCommand = NULL;
		}
	}
	else
	{
		FxVM::DisplayInfo("undo: no more operations to undo", FxTrue);
	}
}

void FxUndoRedoManager::Redo( FxBool noUndo )
{
	if( _redoStack.Length() > 0 )
	{
		FxCommand* pCommand = _redoStack.Back();
		_redoStack.PopBack();
		pCommand->Redo();
		FxString redoString = "redo ";
		redoString = FxString::Concat(redoString, pCommand->GetCommandString());
		FxVM::DisplayEcho(redoString);
		if( !noUndo )
		{
			_undoStack.PushBack(pCommand);
		}
		else
		{
			delete pCommand;
			pCommand = NULL;
		}
	}
	else
	{
		FxVM::DisplayInfo("redo: no more operations to redo", FxTrue);
	}
}

FxBool FxUndoRedoManager::CommandsToUndo( void )
{
	if( _undoStack.Length() > 0 )
	{
		return FxTrue;
	}
	return FxFalse;
}

FxBool FxUndoRedoManager::CommandsToRedo( void )
{
	if( _redoStack.Length() > 0 )
	{
		return FxTrue;
	}
	return FxFalse;
}

void FxUndoRedoManager::PushUndoableCommand( FxCommand* pCommand )
{
	_undoStack.PushBack(pCommand);
	// Flush the redo stack.
	if( _flushRedoStack )
	{
		for( FxSize i = 0; i < _redoStack.Length(); ++i )
		{
			delete _redoStack[i];
		}
		_redoStack.Clear();
	}
}

void FxUndoRedoManager::Flush( void )
{
	for( FxSize i = 0; i < _undoStack.Length(); ++i )
	{
		delete _undoStack[i];
	}
	_undoStack.Clear();
	for( FxSize i = 0; i < _redoStack.Length(); ++i )
	{
		delete _redoStack[i];
	}
	_redoStack.Clear();
}

void FxUndoRedoManager::SetFlushRedoStackOnExecute( FxBool flushRedoStack )
{
	_flushRedoStack = flushRedoStack;
}

void FxUndoRedoManager::DumpUndoStack( void )
{
	FxVM::DisplayInfo("Undo stack dump:");
	FxVM::DisplayInfo("----------------");
	for( FxSize i = 0; i < _undoStack.Length(); ++i )
	{
		if( _undoStack[i] )
		{
			FxVM::DisplayInfo(_undoStack[i]->GetCommandString());
		}
	}
	FxVM::DisplayInfo("----------------");
}

void FxUndoRedoManager::DumpRedoStack( void )
{
	FxVM::DisplayInfo("Redo stack dump:");
	FxVM::DisplayInfo("----------------");
	for( FxSize i = 0; i < _redoStack.Length(); ++i )
	{
		if( _redoStack[i] )
		{
			FxVM::DisplayInfo(_redoStack[i]->GetCommandString());
		}
	}
	FxVM::DisplayInfo("----------------");
}

} // namespace Face

} // namespace OC3Ent
