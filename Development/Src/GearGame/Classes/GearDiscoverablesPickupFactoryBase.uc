/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearDiscoverablesPickupFactoryBase extends GearPickupFactory
	abstract
	dependson(GearProfileSettings)
	showcategories(Display)
	native;


/************************************************************************/
/* Constants															*/
/************************************************************************/

/** Enum of the different methods the player will use to grab the discoverable (tied to animations) */
enum EGearDiscoverablePickupType
{
	eGDPT_Ground,
	eGDPT_Wall,
};

/************************************************************************/
/* Member variables                                                     */
/************************************************************************/

/** The actual discoverable this factory is handling */
var() EGearDiscoverableType	DISC_DiscoverableType;

/** The type of method the player will use to grab the discoverable (tied to animations) */
var() EGearDiscoverablePickupType DISC_PickupType;

/** Sound to play when discoverable is picked up */
var() SoundCue DISC_PickupSound;

/** The mesh that will be rendered to represent this discoverable */
var() editconst StaticMeshComponent DISC_MeshComponent;

/** The background texture to display when this discoverable is picked up */
var() Texture2D DISC_BackgroundTexture;

var transient HeadTrackTargetSpawnable	HeadTrackTarget;

var() protected const float GUDSHintRadius;

/** TRUE if we already did the guds hint.  Transient so it can be repeated in a later game session. */
var protected transient bool bDidGUDSHint;

simulated function bool CanPickUpAmmoType(Pawn P, class<GearAmmoType> AmmoTypeClass, bool bExcludeChecks)
{
	return FALSE;
}

function bool PickUpAmmoFromWeaponClass(Pawn P, class<GearWeapon> WeaponClass)
{
	return FALSE;
}

/************************************************************************/
/* Script functions                                                     */
/************************************************************************/

simulated function PostBeginPlay()
{
	super.PostBeginPlay();

	if (GUDSHintRadius > 0.f)
	{
		HeadTrackTarget = Spawn(class'HeadTrackTargetSpawnable', self);
		if (HeadTrackTarget != None)
		{
			HeadTrackTarget.SetBase(self);
			HeadTrackTarget.SetHardAttach(TRUE);
			HeadTrackTarget.SetRelativeLocation(PickupMesh.Translation);
			HeadTrackTarget.PawnFilter = HT_PlayersOnly;
			HeadTrackTarget.Radius = GUDSHintRadius;
		}
	}

	//DebugBreak();
}


simulated function InitializePickup()
{
	SetPickupMesh();
}

simulated function SetPickupMesh()
{
	if ( DISC_MeshComponent != None )
	{
		if ( bPickupHidden )
		{
			SetPickupHidden();
		}
		else
		{
			SetPickupVisible();
		}
	}
}

/** Overloaded so we can determine whether or not to show this discoverable or not */
simulated event SetInitialState()
{
	// note: not calling super function here on purpose, so this needs to be here
	bScriptInitialized = TRUE;

	if ( DISC_DiscoverableType != eDISC_None )
	{
		CheckForDiscoverableVisibility();
	}
	// LD didn't set the DiscoverableType
	else
	{
		`log("Discoverable"@self@"does not have DISC_DiscoverableType set properly!");
	}
}

/** Loops through local player controllers seeing if we should show the discoverable or not */
simulated function CheckForDiscoverableVisibility()
{
	local GearPC APlayer;
	local bool bNeedDiscoverable, bFoundControllers;

	// search for any local players that need the discoverable
	ForEach LocalPlayerControllers(class'GearPC', APlayer)
	{
		bFoundControllers = TRUE;
		bNeedDiscoverable = (bNeedDiscoverable || !APlayer.HasFoundDiscoverable( DISC_DiscoverableType ));
	}

	SetVisible( bNeedDiscoverable || !bFoundControllers );
}

/** Toggles, locally, discoverable visibility */
simulated function SetVisible(bool bNewVisible)
{
	if ( bNewVisible )
	{
		GotoState('Auto');
	}
	else
	{
		GotoState('disabled');
	}
}

/** See if we can do something with this discoverable pickup */
simulated function Name FindInteractionWith(Pawn P, bool bExcludeChecks)
{
	// using neither 'Pickup' || 'SwapWithCurrent'  will cause GearSpecialMove's SpecialMoveStarted to play BS_Pickup
	return 'Grab';
}

/** Give pickup to player */
simulated function GiveTo(Pawn P)
{
	local GearPC GPC;

	if ( DISC_DiscoverableType != eDISC_None )
	{
		GPC = GearPC(P.Controller);
		if ( GPC != None )
		{
			// Tell PC that it is to pick up a discoverable
			GPC.PickedUpDiscoverable( self );
		}
	}
	else
	{
		P.ClientMessage("Discoverable"@self@"does not have DISC_DiscoverableType set properly!");
	}
}

/** No respawning */
simulated function SetRespawn()
{
}

/** Sets a timer to tell the discoverable to check for visibility */
simulated function HideDiscoverableAfterDelay( float TimeDelay )
{
	SetTimer( TimeDelay, FALSE, nameof(HideDiscoverable) );
}

/** Tell the discoverable hide */
simulated function HideDiscoverable()
{
	SetVisible( FALSE );
}

/** PICKUP STATE */
auto simulated state Pickup
{
	simulated event BeginState(name PreviousStateName)
	{
		SetHidden(FALSE);
		SetCollision(default.bCollideActors, default.bBlockActors);
		super.BeginState(PreviousStateName);
	}

	simulated event Tick(float DeltaTime)
	{
		Super.Tick(DeltaTime);

		if (InterpActor(Base) != None)
		{
			SetHidden(Base.bHidden);
		}
	}
};

/** DISABLED STATE */
simulated state Disabled
{
	simulated function BeginState(Name PreviousStateName)
	{
		SetHidden(TRUE);
		SetCollision(FALSE, FALSE);
		super.BeginState(PreviousStateName);
	}
}

/** Set the nav logging type */
simulated event string GetDebugAbbrev()
{
	return "DISC";
}

simulated protected event NotifyLookedAt()
{
	bDidGUDSHint = TRUE;
	if (HeadTrackTarget != None)
	{
		SetTimer( 3.f, FALSE, nameof( HeadTrackTarget.DisableSelf), HeadTrackTarget );
	}
}


defaultproperties
{

}
