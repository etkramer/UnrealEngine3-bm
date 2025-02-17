/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTAmmo_Stinger extends UTAmmoPickupFactory;

defaultproperties
{
	AmmoAmount=50
	TargetWeapon=class'UTWeap_Stinger'
	PickupSound=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_Stinger_Cue'
	MaxDesireability=0.25

	Begin Object Name=AmmoMeshComp
		StaticMesh=StaticMesh'Pickups.Ammo_Stinger.Mesh.S_Pickups_Ammo_Stinger'
		Translation=(X=0.0,Y=0.0,Z=-5.0)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=14.4
	End Object
}
