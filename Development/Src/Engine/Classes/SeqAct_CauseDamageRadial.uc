/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CauseDamageRadial extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}

/** Type of damage to apply */
var() class<DamageType>		DamageType;

/** Amount of momentum to apply */
var() float					Momentum;

/** Amount of damage to apply */
var() float			DamageAmount;

/** Distance to Instigator within which to damage actors */
var()	float		DamageRadius;

/** Whether damage should decay linearly based on distance from the instigator. */
var()	bool		bDamageFalloff;

/** player that should take credit for the damage (Controller or Pawn) */
var Actor Instigator;

/**
 * Return the version number for this class.  Child classes should increment this method by calling Super then adding
 * a individual class version to the result.  When a class is first created, the number should be 0; each time one of the
 * link arrays is modified (VariableLinks, OutputLinks, InputLinks, etc.), the number that is added to the result of
 * Super.GetObjClassVersion() should be incremented by 1.
 *
 * @return	the version number for this specific class.
 */
static event int GetObjClassVersion()
{
	return Super.GetObjClassVersion() + 1;
}

defaultproperties
{
	ObjName="Cause Damage Radial"
	ObjCategory="Actor"

	VariableLinks(1)=(ExpectedType=class'SeqVar_Float',LinkDesc="Amount",PropertyName=DamageAmount)
	VariableLinks(2)=(ExpectedType=class'SeqVar_Object',LinkDesc="Instigator",PropertyName=Instigator)
	Momentum=500.f
	DamageRadius=200.f
}
