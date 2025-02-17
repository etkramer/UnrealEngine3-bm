/**
 * This will do the actions needed to help reduce memory footprint before a cine.
 *
 * This allows a centralized location for all Pre Cine actions.
 * 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SeqAct_CineCleanWorldPre extends SequenceAction
	native(Sequence);

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
	`log( "CINE CLEAN WORLD PRE" );
	TurnOffAndFlushGUDs();
	CleanWorldOfDetritus();
	FlushGearObjectPool(); // this will cause a GC to occur so it should always come last in the list of things to do
}

/** This will tell everyone to flish their GUDs and then turn off the GUDsManager's streaming in of new data **/
simulated native function TurnOffAndFlushGUDs();


// duplicated from  SeqAct_CleanWorldOfDetritus
simulated function CleanWorldOfDetritus()
{
	local Actor A;

	foreach GetWorldInfo().DynamicActors( class'Actor', A )
	{
		if( (A.IsA('GearPawn') && A.bTearOff) 
			|| A.IsA('Emit_SmokeGrenade') 
			|| A.IsA('GearFogVolume_SmokeGrenade')
			// || A.IsA('GearDroppedPickup')  NOTE we do not want to do this as this kismet is being used everywhere now so you end up losing weapons after a battle which plays a cine.  Additionally, once players move more than 1024 units away the fadeout timer on dropped weapons kicks in. 
			)
		{
			A.Destroy();
		}
	}
}

// duplicated from  SeqAct_ObjectPoolForceFlush
simulated function FlushGearObjectPool()
{
	local GearPC PC;

	// This will cause the object pool to flush
	foreach GetWorldInfo().AllControllers( class'GearPC', PC )
	{
		PC.ClientObjectPoolForceFlush();
	}
}



defaultproperties
{
	ObjName="Cine Clean World Pre"
	ObjCategory="Cinematic"
	
	VariableLinks.Empty
}
