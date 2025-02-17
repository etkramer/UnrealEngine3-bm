//------------------------------------------------------------------------------
// The cook command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCookCommand_H__
#define FxCookCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The cook command.
class FxCookCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCookCommand, FxCommand);
public:
	// Constructor.
	FxCookCommand();
	// Destructor.
	virtual ~FxCookCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
