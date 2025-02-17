/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_HydraSetDamage extends SequenceAction;

/** Part to set health of */
var()	EHydraDamagedPart	Part;

/** Health to set it to */
var()	int					Health;

defaultproperties
{
	ObjName="Hydra: Set Damage"
	ObjCategory="Boss"

	InputLinks(0)=(LinkDesc="Allow Damage")
	InputLinks(1)=(LinkDesc="Disallow Damage")
	InputLinks(2)=(LinkDesc="Set Health")
}