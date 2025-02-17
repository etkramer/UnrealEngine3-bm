/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAmmo_FlakCannon extends UTAmmoPickupFactory;

defaultproperties
{
	AmmoAmount=8
	TargetWeapon=class'UTWeap_FlakCannon'
	PickupSound=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_Flak_Cue'
	MaxDesireability=0.32

	Begin Object Name=AmmoMeshComp
		StaticMesh=StaticMesh'Pickups.Ammo_Flak.Mesh.S_Ammo_Flakcannon'
		Scale=2.0
		Translation=(X=0.0,Y=0.0,Z=-6.0)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=14.4
	End Object
}
