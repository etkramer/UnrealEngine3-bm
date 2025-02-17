//------------------------------------------------------------------------------
// The close actor command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCloseActorCommand_H__
#define FxCloseActorCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The close actor command.
class FxCloseActorCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCloseActorCommand, FxCommand);
public:
	// Constructor.
	FxCloseActorCommand();
	// Destructor.
	virtual ~FxCloseActorCommand();

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
