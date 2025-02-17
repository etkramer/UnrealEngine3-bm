//------------------------------------------------------------------------------
// The set name command, for renaming specific objects in an actor.
// 
// Owner: John Briggs
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSetNameCommand_H__
#define FxSetNameCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

class FxSetNameCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxSetNameCommand, FxCommand);
public:
	// Constructor.
	FxSetNameCommand();
	// Destructor.
	virtual ~FxSetNameCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	FxBool _faceGraphNode;
	FxBool _animGroup;
	FxBool _anim;
	FxBool _animCurve;

	FxString _oldName;
	FxString _newName;

	FxArray<FxString> _path;
};

} // namespace Face

} // namespace OC3Ent

#endif
