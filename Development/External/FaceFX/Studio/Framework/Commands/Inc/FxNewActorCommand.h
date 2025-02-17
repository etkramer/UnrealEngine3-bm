//------------------------------------------------------------------------------
// The new actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxNewActorCommand_H__
#define FxNewActorCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The new actor command.
class FxNewActorCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxNewActorCommand, FxCommand);
public:
	// Constructor.
	FxNewActorCommand();
	// Destructor.
	virtual ~FxNewActorCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
