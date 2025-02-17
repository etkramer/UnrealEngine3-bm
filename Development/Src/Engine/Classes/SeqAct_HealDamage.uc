/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_HealDamage extends SequenceAction;

/** Type of healing to apply */
var() class<DamageType> DamageType;

/** Amount of healing to apply */
var() int HealAmount;

/** player that should take credit for the healing (Controller or Pawn) */
var Actor Instigator;

defaultproperties
{
	ObjName="Heal Damage"
	ObjCategory="Actor"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Int',LinkDesc="Amount",PropertyName=HealAmount)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",PropertyName=Instigator)
}
