/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CauseExplosion extends SequenceAction
	native(Sequence);

cpptext
{
	void Activated();
}

/** Defines the explosion. */
var() export editinline GearExplosion	CustomExplosionTemplate;

var() class<GearProj_ExplosiveBase>		ProjectileExplosionToEmulate;

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
	ObjName="Cause Explosion"
	ObjCategory="Gear"
}
