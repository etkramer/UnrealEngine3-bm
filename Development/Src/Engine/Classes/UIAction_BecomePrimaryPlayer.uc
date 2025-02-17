/**
 * Modifies the owning player's position in the GamePlayers array so that the owning player becomes the primary player (located at position
 * 0 in the players array).
 *
 * This action's logic is executed by a handler function.
 *
 * Copyright 2008 Epic Games, Inc. All Rights Reserved
 */
class UIAction_BecomePrimaryPlayer extends UIAction;

DefaultProperties
{
	ObjName="Become Primary Player"
	ObjCategory="Player"

	bAutoTargetOwner=true

	// only allow this action to be performed on one widget at a time
	VariableLinks(0)=(ExpectedType=class'SeqVar_Object',LinkDesc="Target",PropertyName=Targets,MaxVars=1)
}
