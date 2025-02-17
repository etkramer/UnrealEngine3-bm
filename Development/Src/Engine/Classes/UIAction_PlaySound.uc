/**
 * This class allows the PlaySound action to be used in the UI.  The only change in this class is to force the Target array
 * to be a player variable.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UIAction_PlaySound extends SeqAct_PlaySound;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	// this action is allowed in the UI, but won't work unless the designer hooks up a Player variable to the targets array
	return true;
}

DefaultProperties
{
	ObjName="Play Sound UI"

	VariableLinks(0)=(ExpectedType=class'SeqVar_Player',LinkDesc="Target",PropertyName=Targets)
}
