/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearCrimsonOmenPickupFactoryBase extends GearPickupFactory
	abstract
	dependson(GearProfileSettings)
	native;

/** Sound to play when omen is picked up. */
var() SoundCue						TagPickupSound;

/** Exactly which COGTag this is -- there are 3 per game section */
var() GearProfileSettings.ECogTag	TagID;

/** true if initial visibility has been set */
var transient bool					bInitialTagStateSet;

/** TRUE if this tag is visible, false otherwise */
var transient repnotify bool		bVisible;

/** once this flag is true on clients, we know the object has been initally replicated */
var transient bool		bDidInitialReplication;

replication
{
	if (Role == ROLE_Authority)
		bVisible, bDidInitialReplication;
}

simulated event ReplicatedEvent( name VarName )
{
	if (VarName == 'bVisible')
	{
		SetVisible(bVisible);
	}
	//else if (VarName == 'bDidInitialReplication')
	//{
	//	`log("**** CLIENT received bDidInitialReplication="@bDidInitialReplication@self@TagID);
	//}
	else
	{
		super.ReplicatedEvent(VarName);
	}
}

simulated function InitializePickup()
{
	// not need for omens
}

/**
 * Gotta hijack this because PickupFactory::SetInitialState does
 * bad things (forces us to Disabled if InventoryType == None).
 */
simulated event SetInitialState()
{
//	`log("***"@self@TagID@GetFuncName());

	// disabled by default.  We will enable needed tags as necessary in Tick.
	bVisible = FALSE;
	GotoState('Disabled');

	// seems wierd semantically, I know, but this variable indicates when
	// the REAL state has been set, which happens in Tick().
	bInitialTagStateSet = FALSE;

	// note: not calling super function here on purpose, so this needs to be here
	bScriptInitialized = TRUE;

	// once this flag is true on clients, we know the object has been initally replicated
	if (Role == ROLE_Authority)
	{
		bDidInitialReplication = TRUE;
	}

	// danger Will Robinson
	if (TagID == COGTAG_None)
	{
		`log("COGTag"@self@"does not have TagID set properly!");
	}
}

function NotifyCOGTagVisibility(bool bNewIsVisible)
{
//	`log("***"@GetFuncName()@bNewIsVisible@self@TagID);
	// if server thinks tag is off but someone has told us they want it turned on, turn it on
	if ( !bVisible && bNewIsVisible )
	{
		SetVisible(TRUE);
	}
}

/** Toggles, locally, cog tag visibility */
simulated function SetVisible(bool bNewVisible)
{
//	`log("********"@GetFuncName()@bNewVisible@self@TagID);
//	ScriptTrace();

	if (bNewVisible)
	{
		GotoState('Auto');
	}
	else
	{
		GotoState('disabled');
	}

	// triggers visibility change on clients
	if (Role == ROLE_Authority)
	{
		bVisible = bNewVisible;
	}
}

/**
 * See if we can do something with this tag pickup.
 */
simulated function Name FindInteractionWith(Pawn P, bool bExcludeChecks)
{
	// using neither 'Pickup' || 'SwapWithCurrent'  will cause GearSpecialMove's SpecialMoveStarted to play BS_Pickup
	return 'Grab';
}


/**
 * Give pickup to player
 */
function GiveTo(Pawn P)
{
	local GearPC WPC;
	if (TagID != COGTAG_None)
	{
		// cog tag gets picked, everyone gets credit!
		foreach WorldInfo.AllControllers(class'GearPC', WPC)
		{
	//		`log(WPC@"picked up COGTag"@TagID);
			GearGame(WorldInfo.Game).TriggerGUDEvent(GUDEvent_PickedUpCollectible, P, None, 0.5f);
		}
	}
	else
	{
		P.ClientMessage("COGTag"@self@"does not have TagID set properly!");
	}

	PickedUpBy(P);
}

//
// Set up respawn waiting if desired.
//
function SetRespawn()
{
	// no respawning, just turn it off.
	// since everyone gets it if one person picks it up, we can just trivially turn it off
	SetVisible(FALSE);
}

simulated function bool CanBePickedUpBy(Pawn P)
{
	return FALSE;
}

auto simulated state Pickup
{
	simulated event BeginState(name PreviousStateName)
	{
		TriggerEventClass(class'SeqEvent_PickupStatusChange', None, 0);

		if (PreviousStateName == 'Disabled')
		{
			SetHidden(FALSE);
			SetCollision(TRUE, TRUE);
		}

		super.BeginState(PreviousStateName);
	}
};

simulated state Disabled
{
	simulated function BeginState(Name PreviousStateName)
	{
		super.BeginState(PreviousStateName);
	}
}

simulated event string GetDebugAbbrev()
{
	return "COF";
}

defaultproperties
{

}
