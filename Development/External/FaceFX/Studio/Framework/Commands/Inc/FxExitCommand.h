//------------------------------------------------------------------------------
// The exit command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxExitCommand_H__
#define FxExitCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The exit command.
class FxExitCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxExitCommand, FxCommand);
public:
	// Constructor.
	FxExitCommand();
	// Destructor.
	virtual ~FxExitCommand();

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
