//------------------------------------------------------------------------------
// The select command.
//
// Owner: Jamie Redmond
//
// Copyright (c) 2002-2008 OC3 Entertainment, Inc.
//------------------------------------------------------------------------------

#ifndef FxSelectCommand_H__
#define FxSelectCommand_H__

#include "FxCommand.h"

namespace OC3Ent
{

namespace Face
{

// The select command.
class FxSelectCommand : public FxCommand
{
	// Declare the class.
	FX_DECLARE_CLASS(FxSelectCommand, FxCommand);
public:
	// Constructor.
	FxSelectCommand();
	// Destructor.
	virtual ~FxSelectCommand();

	// Sets up the argument syntax.
	static FxCommandSyntax CreateSyntax( void );

	// Execute the command.
	virtual FxCommandError Execute( const FxCommandArgumentList& argList );

	// Undoes the command.
	virtual FxCommandError Undo( void );
	// Redoes the command.
	virtual FxCommandError Redo( void );

protected:
	// The type of object to select.
	FxString _type;
	// A flag telling the command to clear the current selection.
	FxBool _clear;
	// A flag telling the command whether or not to zoom to the selection.
	FxBool _zoomToSelection;
	// A flag describing if the object is to be added to the current selection.
	FxBool _addToCurrent;
	// A flag describing if the object is to be removed from the current selection.
	FxBool _removeFromCurrent;
	// The object names being operated on.
	FxArray<FxString> _objects;
	// The optional object group.
	FxString _group;
	// The first node of the link to select.
	FxString _node1;
	// The first second node of the link to select.
	FxString _node2;
	// The previously selected object(s) of this type.
	FxArray<FxString> _prevSelection;
	// The previously selected animation, if the selection is a anim group.
	FxString _prevAnim;
	// The previously selected object group.
	FxString _prevGroup;
	// The previously selected first node of a link.
	FxString _prevNode1;
	// The previously selected second node of a link.
	FxString _prevNode2;
	// The previously selected curves.
	FxArray<FxString> _prevCurves;
	// The previously selected keys.
	FxArray<FxSize> _curveIndices;
	FxArray<FxSize> _keyIndices;
};

} // namespace Face

} // namespace OC3Ent

#endif
