/**
 * Kantus Locust Burst Pistol
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_LocustBurstPistol_Kantus extends GearWeap_LocustBurstPistol;

function bool VerifyGrenadeWeapon()
{
	local GearWeap_GrenadeBase GrenBase;
	local GearPawn P;

	P = GearPawn(Instigator);
	if( GrenWeap == None && P != None )
	{
		foreach InvManager.InventoryActors( class'GearWeap_GrenadeBase', GrenBase )
		{
			GrenWeap = GrenBase;
			break;
		}
	}

	if( GrenWeap != None )
	{
		GrenWeap.AttachWeaponTo( P.Mesh, GetGrenadeSocketName(P) );
	}

	return (GrenWeap != None);
}

function Name GetGrenadeSocketName( GearPawn P )
{
	return P.LeftHandSocketName;
}

function ForceThrowGrenade( GearAI AI )
{
	if( VerifyGrenadeWeapon() )
	{
		GrenWeap.ForceThrowGrenade( AI );
	}
}

// don't want it to turn IK on when targeting.
simulated function UpdatePistolLeftHandIK()
{
	local GearPawn P;

	P = GearPawn(Instigator);

	// When left hand should be IK'd
	// When doing 360 aiming, when targeting, when reloading weapon
	if( P.IsAKidnapper() || P.bDoing360Aiming || P.IsReloadingWeapon() )
	{
		bDisableLeftHandIK = FALSE;
		P.UpdateBoneLeftHandIK();
	}
	else
	{
		bDisableLeftHandIK = TRUE;
		P.UpdateBoneLeftHandIK();
	}
}

defaultproperties
{
	bAllowDownsightsStance=FALSE
	DroppedWeaponClass=class'GearWeap_LocustBurstPistol'
}
