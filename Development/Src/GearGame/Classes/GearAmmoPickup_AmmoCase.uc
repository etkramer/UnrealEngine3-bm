
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearAmmoPickup_AmmoCase extends GearPickupFactory
	config(Weapon);

var localized String DisplayName;

var() bool bInfiniteAmmo;

var const Vector2D NumMagsDefault;

var config Vector2D NumMagsAssaultRifle;
var config Vector2D NumMagsBoomShot;
var config Vector2D NumMagsGrenadeFrag;
var config Vector2D NumMagsGrenadeSmoke;
var config Vector2D NumMagsLocustAssaultRifle;
var config Vector2D NumMagsCOGPistol;
var config Vector2D NumMagsLocustPistol;
var config Vector2D NumMagsShotgun;
var config Vector2D NumMagsSniperRifle;
var config Vector2D NumMagsTorqueBow;
var config Vector2D NumMagsFlamethrower;
var config Vector2D NumMagsLocustBurstPistol;

/** Bypass PickupFactory implementation. We do not have InventoryType set. */
simulated event SetInitialState()
{
	if (Role == ROLE_Authority && WorldInfo.GRI.IsMultiplayerGame() && !WorldInfo.Game.IsA('GearGameHorde_Base'))
	{
		GotoState('Disabled');
	}
	else
	{
		Super(Actor).SetInitialState();
	}
}

/**
* returns the display name of the item to be picked up
*/
simulated function String GetDisplayName()
{
	return DisplayName;
}

/** Set default ammo mesh. */
simulated function SetPickupMesh();
/** We're not using InventoryType class like PickupFactory.uc */
simulated function InitializePickup();

/**
 * bExcludeChecks - TRUE = could you have?
 *                - FALSE = you actually can
 */
simulated function bool FindPickupAmmoTypes(Pawn P, optional out array<class<GearAmmoType> > out_AmmoTypes, optional bool bExcludeChecks)
{
	local GearInventoryManager InvManager;
	local GearWeapon Weap;

	InvManager = GearInventoryManager(P.InvManager);

	if (InvManager != None)
	{
		foreach InvManager.InventoryActors(class'GearWeapon',Weap)
		{
			if ( Weap.CharacterSlot != EASlot_None
				&& (bExcludeChecks || (Weap.GetMaxSpareAmmoSize() > 0 && Weap.SpareAmmoCount < Weap.GetMaxSpareAmmoSize())) &&
				GetNumMagsBasedOnAmmoType(Weap.AmmoTypeClass).Y > 0.0 )
			{
				out_AmmoTypes[out_AmmoTypes.Length] = Weap.AmmoTypeClass;
			}
		}
	}
	return (out_AmmoTypes.Length > 0);
}

/**
 * See if we can do something with this weapon pickup.
 * bExcludeChecks - TRUE = could you have?
 *                - FALSE = you actually can
 */
simulated function Name FindInteractionWith(Pawn P, bool bExcludeChecks)
{
	local GearPawn GP;
	GP = GearPawn(P);
	// Don't allow any pickup when carrying crate or when picking something up
	if (bHidden || GP == None || GP.CarriedCrate != None || GP.IsDoingSpecialMove(SM_WeaponPickup) || !FindPickupAmmoTypes(P,,bExcludeChecks))
	{
		return 'None';
	}
	return 'TakeAmmo';
}


/**
 * Give pickup to player
 */
function GiveTo(Pawn P)
{
	local array<class<GearAmmoType> > AmmoTypes;
	local int Idx;
	local float Amount;
	local GearInventoryManager InvManager;
	local GearPawn WP;
	local Vector2D AmmoRng;

	InvManager = GearInventoryManager(P.InvManager);
	WP = GearPawn(P);

	if (InvManager != None && FindPickupAmmoTypes(P,AmmoTypes,false))
	{
		if(WP != none)
		{
			WP.AmmoPickupSound = SoundCue'Weapon_AssaultRifle.Weapons.GenericAmmoPickup01Cue';
		}
		for (Idx = 0; Idx < AmmoTypes.Length; Idx++)
		{
			AmmoRng = GetNumMagsBasedOnAmmoType(AmmoTypes[Idx]);
			if (AmmoRng.Y > 0.0)
			{
				Amount = RandRange(AmmoRng.X,AmmoRng.Y);
				if (AmmoTypes[Idx].default.bRoundAmmoBoxClips)
				{
					// round to int as some weapons don't have partial clips
					Amount = Round(Amount);
				}
				if (AmmoTypes.length == 1)
				{
					// if this is the only ammo type we can get, make sure we get something
					Amount = Max(Amount, 1.0);
				}
				if (Amount > 0.0)
				{
					// bonus ammo on casual
					if (GearPRI(WP.PlayerReplicationInfo).Difficulty == class'DifficultySettings_Casual')
					{
						Amount *= 1.2;
					}
					InvManager.AddAmmoFromAmmoType(AmmoTypes[Idx], Amount);
				}
			}
		}
		if (!bInfiniteAmmo)
		{
			if (WorldInfo.Game.IsA('GearGameHorde_Base'))
			{
				SetHidden(true);
				SetTimer( 45.0, false, nameof(UnHideAmmo) );
			}
			else
			{
				ShutDown();
			}
		}
		else
		{
			SetHidden(TRUE);
			SetTimer( 10.f,FALSE,nameof(UnHideAmmo) );
		}
	}
}

function UnHideAmmo()
{
	ClaimedBy = None;
	bForceNetUpdate = TRUE;
	SetHidden(FALSE);
}

/** This will return the NumMags based on ammo type **/
simulated function Vector2D GetNumMagsBasedOnAmmoType( class<GearAmmoType> TheAmmoType )
{
	if( class'GearAmmoType_AssaultRifle' == TheAmmoType )
	{
		return NumMagsAssaultRifle;
	}
	else if( class'GearAmmoType_Boomer' == TheAmmoType )
	{
		return NumMagsBoomShot;
	}
	else if( class'GearAmmoType_GrenadeFrag' == TheAmmoType )
	{
		return NumMagsGrenadeFrag;
	}
	else if( class'GearAmmoType_GrenadeSmoke' == TheAmmoType )
	{
		return NumMagsGrenadeSmoke;
	}
	else if( class'GearAmmoType_LocustAssaultRifle' == TheAmmoType )
	{
		return NumMagsLocustAssaultRifle;
	}
	else if( class'GearAmmoType_Pistol' == TheAmmoType )
	{
		return NumMagsCOGPistol;
	}
	else if( class'GearAmmoType_PistolHighPowered' == TheAmmoType )
	{
		return NumMagsLocustPistol;
	}
	else if( class'GearAmmoType_Shotgun' == TheAmmoType )
	{
		return NumMagsShotgun;
	}
	else if( class'GearAmmoType_SniperRifle' == TheAmmoType )
	{
		return NumMagsSniperRifle;
	}
	else if( class'GearAmmoType_TorqueBow' == TheAmmoType )
	{
		return NumMagsTorqueBow;
	}
	else if( class'GearAmmoType_Flamethrower' == TheAmmoType )
	{
		return NumMagsFlamethrower;
	}
	else if( class'GearAmmoType_GrenadeInk' == TheAmmoType )
	{
		// no ammo evar!
		return vect2d(0,0);
	}
	else if (class'GearAmmoType_LocustBurstPistol' == TheAmmoType)
	{
		return NumMagsLocustBurstPistol;
	}
	else
	{
		return NumMagsDefault;
	}
}

simulated event string GetDebugAbbrev()
{
	return "APCase";
}

defaultproperties
{
	bBlockActors=false

	Components.Remove(Sprite)

	Begin Object Class=StaticMeshComponent Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AmmoPickups.Mesh.Ammopickup_Box'
		bCastDynamicShadow=FALSE
		bOwnerNoSee=TRUE
		CollideActors=FALSE
		Scale=1.0
		Translation=(Z=-20)
		LightEnvironment=PickupLightEnvironment
	End Object
	PickupMesh=StaticMeshComponent0
	Components.Add(StaticMeshComponent0)

	NumMagsDefault=(X=1.5f,Y=3.f)
}
