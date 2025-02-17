//------------------------------------------------------------------------------
// The rename actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRenameActorCommand_H__
#define FxRenameActorCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The rename actor command.
class FxRenameActorCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxRenameActorCommand, FxCommand);
public:
	// Constructor.
	FxRenameActorCommand();
	// Destructor.
	virtual ~FxRenameActorCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	// The previous actor name.
	FxString _previousName;
	// The requested actor name.
	FxString _requestedName;
};

} // namespace Face

} // namespace OC3Ent

#endif
