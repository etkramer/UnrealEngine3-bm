//------------------------------------------------------------------------------
// The current time command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxCurrentTimeCommand_H__
#define FxCurrentTimeCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The current time command.
class FxCurrentTimeCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxCurrentTimeCommand, FxCommand);
public:
	// Constructor.
	FxCurrentTimeCommand();
	// Destructor.
	virtual ~FxCurrentTimeCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	// The previous time.
	FxReal _previousTime;
	// The requested time.
	FxReal _requestedTime;
	// The requested frame.
	FxInt32 _requestedFrame;
	// True if we're operating in frame mode.
	FxBool _frameMode;
};

} // namespace Face

} // namespace OC3Ent

#endif
