/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_ToggleButtonMash extends SequenceAction;

/** The button type to show. */
var() eGearKismetIconType IconToDisplay;

/** Animation speed for the types of kismet icons that support animating */
var() float AnimationSpeed;

defaultproperties
{
	ObjName="Toggle Button Icon"
	ObjCategory="Gear"

	InputLinks(0)=(LinkDesc="Start")
	InputLinks(1)=(LinkDesc="Stop")

	AnimationSpeed=0.1f
}
