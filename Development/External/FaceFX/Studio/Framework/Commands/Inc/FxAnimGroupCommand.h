//------------------------------------------------------------------------------
// The animation group command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimGroupCommand_H__
#define FxAnimGroupCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The animation group command.
class FxAnimGroupCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnimGroupCommand, FxCommand);
public:
	// Constructor.
	FxAnimGroupCommand();
	// Destructor.
	virtual ~FxAnimGroupCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	// FxTrue if the command is in create mode.
	FxBool _create;
	// FxTrue if the command is in remove mode.
	FxBool _remove;
	// The group name.
	FxString _group;
};

} // namespace Face

} // namespace OC3Ent

#endif
