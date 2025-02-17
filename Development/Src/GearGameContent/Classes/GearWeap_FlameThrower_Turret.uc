/**
 * Flamethrower!
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearWeap_FlameThrower_Turret extends GearWeap_FlameThrower;

simulated function SetWeaponMesh()
{
	WeaponMesh = Instigator.Mesh;
	MuzzleSocketName=Gearpawn_SecurityBotStationaryBase(Instigator).PhysicalFireLocBoneName;
}

defaultproperties
{
	bHidden=true
	// Weapon Mesh
	Begin Object Name=WeaponMesh
		SkeletalMesh=none
		PhysicsAsset=none
	End Object
	bInfiniteSpareAmmo=true
	bNoAnimDelayFiring=true
}



