/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */


class GearPointOfInterest_Meatflag extends GearPointOfInterest
	config(Game)
	native
	nativereplication;


/** The team index of the team who should receive this POI */
var transient int KidnapperTeamIndex;


/************************************************************************/
/* C++ functions                                                        */
/************************************************************************/
cpptext
{
	// Networking
	INT* GetOptimizedRepList( BYTE* InDefault, FPropertyRetirement* Retire, INT* Ptr, UPackageMap* Map, UActorChannel* Channel );
}

/************************************************************************/
/* Replication block                                                    */
/************************************************************************/
replication
{
	if (bNetDirty)
		KidnapperTeamIndex;
}

/** Whether the controller should add/remove this POI */
simulated function bool ShouldControllerHavePOI( GearPC GearPCToTest )
{
	local GearPawn MyGearPawn;

	// See if the kidnapping team is on this controller's team
	if ( GearPCToTest != None &&
		 GearPCToTest.PlayerReplicationInfo != None &&
		 GearPCToTest.PlayerReplicationInfo.GetTeamNum() == KidnapperTeamIndex &&
		 WorldInfo.GRI != None &&
		 GearGRI(WorldInfo.GRI).MeatflagPawn != None )
	{
		// Make sure we don't show it if this is the actual kidnapper
		MyGearPawn = GearPawn(GearPCToTest.Pawn);
		if ( MyGearPawn != None &&
			 MyGearPawn.IsAKidnapper() &&
			 MyGearPawn.InteractionPawn == GearGRI(WorldInfo.GRI).MeatflagPawn )
		{
			return false;
		}

		return true;
	}

	return false;
}

simulated function InitializePOI()
{
	if (!bIsInitialized)
	{
		if (bEnabled)
		{
			EnablePOI();
		}

		bIsInitialized = TRUE;
	}
}

simulated event SetEnabled( bool bOn )
{
	if ( !bOn )
	{
		KidnapperTeamIndex = INDEX_NONE;
	}

	Super.SetEnabled( bOn );
}

defaultproperties
{
	KidnapperTeamIndex=INDEX_NONE
}

