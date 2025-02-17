/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAmmo_BioRifle_Content extends UTAmmoPickupFactory;

defaultproperties
{
	AmmoAmount=20
	TargetWeapon=class'UTWeap_BioRifle_Content'
	PickupSound=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_CGBio_Cue'
	MaxDesireability=0.32

	Begin Object Name=AmmoMeshComp
		StaticMesh=StaticMesh'Pickups.Ammo_Bio.Mesh.S_Pickups_Ammo_Bio'
		Scale=1.875
		Translation=(X=0.0,Y=0.0,Z=0)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=14.4
	End Object

}
