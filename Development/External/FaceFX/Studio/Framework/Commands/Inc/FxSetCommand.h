//------------------------------------------------------------------------------
// The set command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSetCommand_H__
#define FxSetCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The set command.
class FxSetCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxSetCommand, FxCommand);
public:
	// Constructor.
	FxSetCommand();
	// Destructor.
	virtual ~FxSetCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
