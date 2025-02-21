/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPointOfInterest extends KeyPoint
	config(Game)
	native
	nativereplication;


/************************************************************************/
/* Constants, Enums, Structs, etc.										*/
/************************************************************************/

/** Preset priority values for POIs */
var const int POIPriority_ScriptedEvent;
var const int POIPriority_RevivableComrade;
var const int POIPriority_MoveOrder;
var const int POIPriority_TargetOrder;
var const int POIPriority_ComradeHuman;
var const int POIPriority_Comrade;
var const int POIPriority_Pickup;

/** Enum of the different ForceLook types */
enum EPOIForceLookType
{
	ePOIFORCELOOK_None,
	ePOIFORCELOOK_Automatic,
	ePOIFORCELOOK_PlayerInduced,
};

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** -------Begin list of variables that are copied from the SeqAct_ManagePOI------- */

/** The name of the lookup variable for finding the localized text in the [POIS] section of the GearGame.int file */
var string DisplayName;

/** Whether the POI is enabled or not (on, off) */
var repnotify bool bEnabled;

/** The length of time the Y-Look icon will show on the screen when this POI is enabled */
var config float IconDuration;

/** Whether this POI will force the player to look at it or not */
var EPOIForceLookType ForceLookType;

/** The length of time the ForceLook will happen, if ForceLookType is set */
var float ForceLookDuration;

/** Whether of not a line of sight check should occur when doing a ForceLook, if ForceLookType is set */
var bool bForceLookCheckLineOfSight;

/** All POIs have a priority, and this is the LD's way of overriding that priority (should only be used when absolutely necessary) */
var config int LookAtPriority;

/** Field of view to zoom to when the player looks at the POI */
var config float DesiredFOV;

/** The number of times the POI will perform the POI_DesiredFOV zoom when the player looks at it - 0 means infinite */
var int FOVCount;

/** Whether or not to do a line of sight check when performing the POI_DesiredFOV zoom when the player looks at the POI */
var config bool bDoTraceForFOV;

/** Whether or not this POI should force all other active scripted POIs to disable themselves */
var bool bDisableOtherPOIs;

/** The length of time this POI will remain enabled - the POI will auto disable afterward */
var config float EnableDuration;

var bool bLeavePlayerFacingPOI;

/** -------End list of variables that are copied from the SeqAct_ManagePOI------- */


/** The current duration left on the icon */
var float CurrIconDuration;

/** The actor this POI is attached to */
var repnotify Actor AttachedToActor;

/** Whether this POI is initialized or not */
var protected bool bIsInitialized;

/** The potential kismet action that could be referencing this POI */
var SeqAct_ManagePOI POIAction;


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
		bEnabled, AttachedToActor, IconDuration, DisplayName, ForceLookType, DesiredFOV, FOVCount,
		bDoTraceForFOV, ForceLookDuration, LookAtPriority, bForceLookCheckLineOfSight;
}

/** replicated event */
simulated event ReplicatedEvent( name VarName )
{
	switch( VarName )
	{
		case 'AttachedToActor':
			InitializePOI();
			break;
		case 'bEnabled':
			if (bEnabled)
			{
				EnablePOI();
			}
			else
			{
				DisablePOI();
			}
			break;
		default:
			break;
	}
}


/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

simulated event SetEnabled( bool bOn )
{
	local bool OldbEnabled;

	// Store old value
	OldbEnabled = bEnabled;

	// Set the flag
	if ( bOn )
	{
		bEnabled = TRUE;
	}
	else
	{
		bEnabled = FALSE;
	}

	// If flag is a new value force the net to update
	if ( OldbEnabled != bEnabled )
	{
		bForceNetUpdate = TRUE;
	}

	// Call the appropriate function
	if ( bEnabled )
	{
		EnablePOI();
	}
	else
	{
		DisablePOI();
	}
}

/** Whether the controller should add/remove this POI */
simulated function bool ShouldControllerHavePOI( GearPC GearPCToTest )
{
	return true;
}

simulated function EnablePOI()
{
	local GearPC LPC;

	if (Role == ROLE_Authority)
	{
		// Set the duration timer
		if ( EnableDuration > 0 )
		{
			SetTimer( EnableDuration, false, nameof(DisablePOI) );
		}
	}

	CurrIconDuration = IconDuration;

	foreach LocalPlayerControllers(class'GearPC', LPC)
	{
		if ( ShouldControllerHavePOI(LPC) )
		{
			LPC.AddPointOfInterest( self );
		}
	}
}

simulated function DisablePOI()
{
	local GearPC LPC;

	// Clear the timer if and set variables if needed
	if ( Role == ROLE_Authority )
	{
		if ( IsTimerActive('DisablePOI') )
		{
			ClearTimer( 'DisablePOI' );
		}

		bEnabled = FALSE;
		bForceNetUpdate = TRUE;

		if ( POIAction != None )
		{
			POIAction.bIsDone = TRUE;
		}
	}

	foreach LocalPlayerControllers(class'GearPC', LPC)
	{
		LPC.RemovePointOfInterest( self, ForceLookType );
	}
}

/** Internal. */
simulated protected function AttachToActor( Actor Host )
{
	AttachedToActor = Host;
	bForceNetUpdate = TRUE;

	if ( AttachedToActor != None )
	{
		if ( AttachedToActor.IsA('GearDroppedPickup') || AttachedToActor.IsA('GearPickupFactory') || AttachedToActor.IsA('GearPawn') )
		{
			SetBase( AttachedToActor );
		}
	}
}

/*
 * Get the priority value for force looks... -1 is returned if no priority could be found.
 */
simulated function int GetLookAtPriority( GearPC PC )
{
	if ( (PC != None) && (AttachedToActor != None) )
	{
		if ( AttachedToActor.IsA('GearDroppedPickup') || AttachedToActor.IsA('GearPickupFactory') )
		{
			if ( VSizeSq(Location - PC.Pawn.Location) > 262144 )
			{
				return -1;
			}
		}
		else if ( AttachedToActor.IsA('GearPawn') )
		{
			return GearPawn(AttachedToActor).GetLookAtPriority( PC, -1 );
		}
	}

	return LookAtPriority;
}

simulated function InitializePOI()
{
	if (!bIsInitialized)
	{
		if (AttachedToActor != None)
		{
			if ( AttachedToActor.IsA('GearDroppedPickup') )
			{
				CurrIconDuration = 0.f;
				IconDuration = 0.f;
				DisplayName = GearDroppedPickup(AttachedToActor).GetDisplayName();
				LookAtPriority = POIPriority_Pickup;
			}
			else if ( AttachedToActor.IsA('GearPickupFactory') )
			{
				CurrIconDuration = 0.f;
				IconDuration = 0.f;
				DisplayName = GearPickupFactory(AttachedToActor).GetDisplayName();
				LookAtPriority = POIPriority_Pickup;
			}
			else if ( AttachedToActor.IsA('GearPawn') )
			{
				if (GearPawn(AttachedToActor).PlayerReplicationInfo != None)
				{
					CurrIconDuration = 0.f;
					IconDuration = 0.f;
					DisplayName = GearPawn(AttachedToActor).PlayerReplicationInfo.PlayerName;
					LookAtPriority = -1;
				}
				else
				{
					// try again in a little bit
					SetTimer( 0.5f,FALSE,nameof(InitializePOI) );
				}
			}
		}

		if (bEnabled)
		{
			EnablePOI();
		}

		bIsInitialized = TRUE;
	}
}

simulated event PostBeginPlay()
{
	CurrIconDuration = IconDuration;
	if ( Role == ROLE_Authority )
	{
		if (Base != None)
		{
			// already attached to something, just record that
			AttachedToActor = Base;
		}
		else
		{
			AttachToActor( Owner );
		}
		bForceNetUpdate = TRUE;

		InitializePOI();
	}
}

/**
*	If LDs put text into the POI themselves we must retrieve that text from the localization system.
*/
native simulated function String RetrievePOIString( String TagName );

simulated function String GetDisplayName()
{
	local String DisplayText;

	// if the displayname is empty, see if we need to display the text based on what the POI is attached to
	if ( len(DisplayName) <= 0 )
	{
		if ( AttachedToActor != None )
		{
			if ( AttachedToActor.IsA('GearDroppedPickup') )
			{
				DisplayText = GearDroppedPickup(AttachedToActor).GetDisplayName();
			}
			else if ( AttachedToActor.IsA('GearPickupFactory') )
			{
				DisplayText = GearPickupFactory(AttachedToActor).GetDisplayName();
			}
			else if ( AttachedToActor.IsA('GearPawn') )
			{
				DisplayText = GearPawn(AttachedToActor).PlayerReplicationInfo.PlayerName;
			}
		}
	}
	// if the displayname is not empty it is a tagname into the localization file, so let's retrieve the string
	else
	{
		DisplayText = RetrievePOIString( DisplayName );
	}

	return DisplayText;
}


simulated function Destroyed()
{
	DisablePOI();
}


/** Returns the FOV this POI desires while being looked at.  Returns <= 0 if it doesn't care. */
final simulated function float GetDesiredFOV(vector CameraLoc)
{
	if ( (FOVCount >= 0) && (DesiredFOV >= 0) )
	{
		// see if we should do a FOV change
		if ( !bDoTraceForFOV || FastTrace(Location, CameraLoc) )
		{
			return DesiredFOV;
		}
	}

	return 0.f;
}

/** Returns the actual world-space position to look at for this POI. */
final simulated function vector GetActualLookatLocation()
{
	local vector FocusLocation;
	local SkeletalMeshComponent ComponentIt;
	local Actor LookatActor;
	local Name LookAtBoneName;
	local GearPawn WP;

	LookAtActor = self;
	if ( AttachedToActor != None )
	{
		WP = GearPawn(AttachedToActor);
		if ( WP != None )
		{
			LookAtBoneName = WP.NeckBoneName;
			LookAtActor = WP;
		}
	}

	if( LookAtBoneName != '' )
	{
		// find a skeletal mesh component on the focus actor
		foreach LookAtActor.ComponentList(class'SkeletalMeshComponent', ComponentIt)
		{
			FocusLocation = ComponentIt.GetBoneLocation(LookAtBoneName);
			break;
		}
	}
	else
	{
		FocusLocation = LookatActor.Location;
	}

	return FocusLocation;
}

defaultproperties
{
	TickGroup=TG_DuringAsyncWork

	bAlwaysRelevant=TRUE
	bReplicateMovement=TRUE

	bStatic=FALSE
	bMovable=TRUE
	RemoteRole=ROLE_SimulatedProxy
	bNoDelete=FALSE

	POIPriority_ScriptedEvent=1000
	POIPriority_RevivableComrade=750
	POIPriority_TargetOrder=700
	POIPriority_MoveOrder=650
	POIPriority_ComradeHuman=-1
	POIPriority_Comrade=-1
	POIPriority_Pickup=500
}
