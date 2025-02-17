//------------------------------------------------------------------------------
// The redo command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRedoCommand_H__
#define FxRedoCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The redo command.
class FxRedoCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxRedoCommand, FxCommand);
public:
	// Constructor.
	FxRedoCommand();
	// Destructor.
	virtual ~FxRedoCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );
    
	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
