//------------------------------------------------------------------------------
// The save actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSaveActorCommand_H__
#define FxSaveActorCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The save actor command.
class FxSaveActorCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxSaveActorCommand, FxCommand);
public:
	// Constructor.
	FxSaveActorCommand();
	// Destructor.
	virtual ~FxSaveActorCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
