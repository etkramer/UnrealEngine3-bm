/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
* This will make all targets uber fighters.  Damage bonuses and whatnot.
*/
class SeqAct_ToggleSuperTroopers extends SequenceAction;

/** Multiplier to set the SuperDamageMultiplier in GearPawns */
var() float SuperDamageMultiplier;

defaultproperties
{
	ObjName="Toggle Super Troopers"
	ObjCategory="Toggle"

	SuperDamageMultiplier=2.f

	InputLinks(0)=(LinkDesc="Enable")
	InputLinks(1)=(LinkDesc="Disable")
}