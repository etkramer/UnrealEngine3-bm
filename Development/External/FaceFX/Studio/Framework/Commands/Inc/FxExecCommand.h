//------------------------------------------------------------------------------
// The exec command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxExecCommand_H__
#define FxExecCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The exec command.
class FxExecCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxExecCommand, FxCommand);
public:
	// Constructor.
	FxExecCommand();
	// Destructor.
	virtual ~FxExecCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

protected:
	// Parses the script file into an array of commands.
	FxBool _parseCommands( const FxString& script,  FxArray<FxString>& commands );
};

} // namespace Face

} // namespace OC3Ent

#endif
