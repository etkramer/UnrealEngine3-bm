//------------------------------------------------------------------------------
// The batch command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxBatchCommand_H__
#define FxBatchCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The begin batch command.
class FxBeginBatchCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxBeginBatchCommand, FxCommand);
public:
	// Constructor.
	FxBeginBatchCommand();
	// Destructor.
	virtual ~FxBeginBatchCommand();

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

// The exec batch command.
class FxExecBatchCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxExecBatchCommand, FxCommand);
public:
	// Constructor.
	FxExecBatchCommand();
	// Destructor.
	virtual ~FxExecBatchCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

// The batch command.
class FxBatchCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxBatchCommand, FxCommand);
public:
	// Constructor.
	FxBatchCommand();
	// Destructor.
	virtual ~FxBatchCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

	// Adds a command to the batch.
	void AddToBatch( const FxString& command );

protected:
	// The commands to batch.
	FxArray<FxString> _batchedCommands;
	// FxTrue if all batched commands were undoable.
	FxBool _allCommandsWereUndoable;
	// FxTrue if Undo has been called at least once.
	FxBool _undoCalled;
	// Booleans for the various flags.
	FxBool _addedNodes;
	FxBool _removedNodes;
	FxBool _editedNodes;
	FxBool _addedCurves;
	FxBool _removedCurves;
	FxBool _mappingChanged;
	FxBool _editedCurves;
};

} // namespace Face

} // namespace OC3Ent

#endif
