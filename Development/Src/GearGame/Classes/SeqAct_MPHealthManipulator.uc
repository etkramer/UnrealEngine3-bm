/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/*****************************************************************************
 This kismet action sets the health and health regeneration of all players
*****************************************************************************/

class SeqAct_MPHealthManipulator extends SequenceAction
	native(Sequence);

/** The value to set all players' health to */
var() int Health;
/** Percent of total health to regenerate per second */
var() float HealthRegenPerSecond;

cpptext
{
	virtual void Activated();
}

defaultproperties
{
	ObjName="MP Health Manipulator"
	ObjCategory="Multiplayer"

	Health=600
	HealthRegenPerSecond=25.f

	InputLinks(0)=(LinkDesc="Set Health")
	InputLinks(1)=(LinkDesc="Set Regeneration")
}
