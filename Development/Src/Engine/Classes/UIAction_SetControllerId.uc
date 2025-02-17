/**
 * Changes the ControllerId for the player that owns the widgets in the targets array.  This action's logic is executed
 * by a handler function.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_SetControllerId extends UIAction;

DefaultProperties
{
	ObjName="Set Controller ID"
	ObjCategory="Player"

	bAutoTargetOwner=true

	// only allow this action to be performed on one widget at a time
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,MaxVars=1)
}
