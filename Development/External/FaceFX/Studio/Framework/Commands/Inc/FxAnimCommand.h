//------------------------------------------------------------------------------
// The animation command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxAnimCommand_H__
#define FxAnimCommand_H__

#include "FxCommand.h"
#include "FxAnim.h"
#include "FxAnimUserData.h"

namespace OC3Ent
{

namespace Face
{

// The animation command.
class FxAnimCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxAnimCommand, FxCommand);
public:
	// Constructor.
	FxAnimCommand();
	// Destructor.
	virtual ~FxAnimCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	// FxTrue if the command is in the add mode.
	FxBool _add;
	// FxTrue if the command is in the remove mode.
	FxBool _remove;
	// FxTrue if the animation to remove is the currently selected animation.
	FxBool _isSelected;
	// FxTrue if the command is in the move mode.
	FxBool _move;
	// FxTrue if the command is in the copy mode.
	FxBool _copy;
	// The animation group.
	FxString _group;
	// The animation name.
	FxString _name;
	// The transfer from group.
	FxString _from;
	// The transfer to group.
	FxString _to;
	// The remembered animation so it can be restored when undoing a remove 
	// operations.
	FxAnim _oldAnim;
	// The remembered animation user data.
	FxAnimUserData* _pOldUserData;
	// FxFalse if the old user data is controlled by the session.
	FxBool _okToDeleteOldUserData;

	// Cached anim group selection
	FxString _cachedAnimGroup;
	// Cached anim selection
	FxString _cachedAnim;
	// Cached curve selection
	FxArray<FxString> _cachedCurves;
	// Cached key selection
	FxArray<FxSize> _cachedCurveIndices;
	FxArray<FxSize> _cachedKeyIndices;
};

} // namespace Face

} // namespace OC3Ent

#endif
