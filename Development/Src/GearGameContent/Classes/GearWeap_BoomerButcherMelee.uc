
/**
 * Base melee weapon
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 *  Weapon used for melee only
 */

class GearWeap_BoomerButcherMelee extends GearWeapon
	config(Weapon)
	hidedropdown;

/** this weapon has no team based colors **/
simulated function SetMaterialBasedOnTeam( MeshComponent TheMesh, int TeamNum );

simulated function TimeWeaponEquipping()
{
	SetTimer( 0.0001f, FALSE, 'WeaponEquipped' );
	return;
}

defaultproperties
{
	WeaponEquipSound=None
	WeaponDeEquipSound=None
	WeaponReloadSound=None
	FireSound=None

	DroppedPickupClass=None
	bPlayIKRecoil=FALSE
	bBlindFirable=FALSE

	bUseMeleeHitTimer=FALSE
	FiringStatesArray(0)=""	// no firing, only melee
}
