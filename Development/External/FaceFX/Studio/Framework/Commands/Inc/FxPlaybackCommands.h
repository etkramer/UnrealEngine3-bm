//------------------------------------------------------------------------------
// Animation playback related commands.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPlaybackCommands_H__
#define FxPlaybackCommands_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

//------------------------------------------------------------------------------
// Play command.
//------------------------------------------------------------------------------

// The play command.
class FxPlayCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxPlayCommand, FxCommand);
public:
	// Constructor.
	FxPlayCommand();
	// Destructor.
	virtual ~FxPlayCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

//------------------------------------------------------------------------------
// Stop command.
//------------------------------------------------------------------------------

// The stop command.
class FxStopCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxStopCommand, FxCommand);
public:
	// Constructor.
	FxStopCommand();
	// Destructor.
	virtual ~FxStopCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
