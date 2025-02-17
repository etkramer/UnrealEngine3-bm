/**
 * This will clean the world of various detritus:
 * dead bodies
 * smoke grenade smoke
 * dropped weapons
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CleanWorldOfDetritus extends SequenceAction;

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


/** This will do the cleaning of the items in the if check **/
event Activated()
{
	local Actor A;

	// duplicated in SeqAct_CineCleanWorldPre
	foreach GetWorldInfo().DynamicActors( class'Actor', A )
	{
		if( (A.IsA('GearPawn') && A.bTearOff) 
			|| A.IsA('Emit_SmokeGrenade') 
			|| A.IsA('GearFogVolume_SmokeGrenade')
			|| A.IsA('GearDroppedPickup') 
			)
		{
			A.Destroy();
		}
	}
}



defaultproperties
{
	ObjName="Clean World Of Detritus"
	ObjCategory="Cinematic"
	
	VariableLinks.Empty
}
