/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAmmo_SniperRifle extends UTAmmoPickupFactory;

defaultproperties
{
	AmmoAmount=10
	TargetWeapon=class'UTWeap_SniperRifle'
	PickupSound=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_Sniper_Cue'

	Begin Object Name=AmmoMeshComp
		StaticMesh=StaticMesh'Pickups.Ammo_Sniper.Mesh.S_Ammo_SniperRifle'
		Scale=1.5
		Translation=(X=0.0,Y=0.0,Z=-15.0)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=14.4
	End Object
}
