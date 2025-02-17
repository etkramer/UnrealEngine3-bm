//------------------------------------------------------------------------------
// The undo command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxUndoCommand_H__
#define FxUndoCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The undo command.
class FxUndoCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxUndoCommand, FxCommand);
public:
	// Constructor.
	FxUndoCommand();
	// Destructor.
	virtual ~FxUndoCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
