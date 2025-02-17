//------------------------------------------------------------------------------
// The load actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxLoadActorCommand_H__
#define FxLoadActorCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The load actor command.
class FxLoadActorCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxLoadActorCommand, FxCommand);
public:
	// Constructor.
	FxLoadActorCommand();
	// Destructor.
	virtual ~FxLoadActorCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
