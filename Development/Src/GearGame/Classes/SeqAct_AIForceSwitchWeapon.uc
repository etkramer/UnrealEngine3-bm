/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class SeqAct_AIForceSwitchWeapon extends SequenceAction;

var() class<GearWeapon> ForcedWeaponType;

var() bool bGoBackToNormalWeaponSelection;

defaultproperties
{
	ObjCategory="AI"
	ObjName="AI Force Switch Weapon"
}
