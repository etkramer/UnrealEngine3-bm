//------------------------------------------------------------------------------
// The node group command.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxNodeGroupCommand_H__
#define FxNodeGroupCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The node group command.
class FxNodeGroupCommand : FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxNodeGroupCommand, FxCommand);

public:
	// Constructor.
	FxNodeGroupCommand();
	// Destructor.
	virtual ~FxNodeGroupCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	enum CommandType
	{
		CreateGroup,
		RemoveGroup,
		AddToGroup,
		RemoveFromGroup,
		SetGroup
	};

	FxBool _selected;

	FxString _previousGroupName;
	FxString _requestedGroupName;

	FxArray<FxString> _previousGroupMembers;
	FxArray<FxString> _requestedGroupMembers;

	CommandType _commandType;
};

} // namespace Face

} // namespace OC3Ent

#endif
