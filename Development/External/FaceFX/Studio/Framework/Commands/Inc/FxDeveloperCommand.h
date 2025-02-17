//------------------------------------------------------------------------------
// The developer command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxDeveloperCommand_H__
#define FxDeveloperCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The developer command.
class FxDeveloperCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxDeveloperCommand, FxCommand);
public:
	// Constructor.
	FxDeveloperCommand();
	// Destructor.
	virtual ~FxDeveloperCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

// A command to eat memory.
class FxEatMemCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxEatMemCommand, FxCommand);
public:
	// Constructor.
	FxEatMemCommand();
	// Destructor.
	virtual ~FxEatMemCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	// The size of the chunk of memory to eat.
	FxSize _memChunkSize;
	// The chunk of memory.
	FxByte* _pMemChunk;
};

} // namespace Face

} // namespace OC3Ent

#endif
