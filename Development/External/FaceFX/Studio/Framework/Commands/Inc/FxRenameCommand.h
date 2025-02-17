//------------------------------------------------------------------------------
// The rename command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxRenameCommand_H__
#define FxRenameCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The rename command.
class FxRenameCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxRenameCommand, FxCommand);
public:
	// Constructor.
	FxRenameCommand();
	// Destructor.
	virtual ~FxRenameCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
