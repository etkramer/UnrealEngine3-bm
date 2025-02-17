//------------------------------------------------------------------------------
// The animSet command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimSetCommand_H__
#define FxAnimSetCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The animSet command.
class FxAnimSetCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnimSetCommand, FxCommand);
public:
	// Constructor.
	FxAnimSetCommand();
	// Destructor.
	virtual ~FxAnimSetCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
