//------------------------------------------------------------------------------
// The print commands.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxPrintCommands_H__
#define FxPrintCommands_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The print command.
class FxPrintCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxPrintCommand, FxCommand);
public:
	// Constructor.
	FxPrintCommand();
	// Destructor.
	virtual ~FxPrintCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

// The warning command.
class FxWarnCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxWarnCommand, FxCommand);
public:
	// Constructor.
	FxWarnCommand();
	// Destructor.
	virtual ~FxWarnCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

// The error command.
class FxErrorCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxErrorCommand, FxCommand);
public:
	// Constructor.
	FxErrorCommand();
	// Destructor.
	virtual ~FxErrorCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );
};

} // namespace Face

} // namespace OC3Ent

#endif
