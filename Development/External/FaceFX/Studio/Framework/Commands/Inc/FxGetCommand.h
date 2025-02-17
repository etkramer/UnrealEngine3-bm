//------------------------------------------------------------------------------
// The get command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxGetCommand_H__
#define FxGetCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The get command.
class FxGetCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxGetCommand, FxCommand);
public:
	// Constructor.
	FxGetCommand();
	// Destructor.
	virtual ~FxGetCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
