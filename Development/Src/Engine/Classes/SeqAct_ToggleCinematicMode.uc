/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleCinematicMode extends SequenceAction;

var() bool bDisableMovement;
var() bool bDisableTurning;
var() bool bHidePlayer;
/** Don't allow input */
var() bool bDisableInput;
/** Whether to hide the HUD during cinematics or not */
var() bool bHideHUD;

/**
 * Determines whether this class should be displayed in the list of available ops in the UI's kismet editor.
 *
 * @param	TargetObject	the widget that this SequenceObject would be attached to.
 *
 * @return	TRUE if this sequence object should be available for use in the UI kismet editor
 */
event bool IsValidUISequenceObject( optional UIScreenObject TargetObject )
{
	return true;
}

defaultproperties
{
	ObjName="Toggle Cinematic Mode"
	ObjCategory="Toggle"

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")
	InputLinks(2)=(LinkDesc="Toggle")

	bDisableMovement=TRUE
	bDisableTurning=TRUE
	bHidePlayer=TRUE
	bDisableInput=TRUE
	bHideHUD=TRUE
}
