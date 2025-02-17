/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class SeqAct_SetWeaponAlert extends SequenceAction;

var() float MaxAlertTime;

defaultproperties
{
	ObjName="Set Weapon Alert"
	ObjCategory="Gear"

	MaxAlertTime=1.f
	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Max Alert Time",PropertyName=MaxAlertTime)
}
