/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class UTAmmo_Enforcer extends UTAmmoPickupFactory;

defaultproperties
{
	AmmoAmount=16
	TargetWeapon=class'UTWeap_Enforcer'
	PickupSound=SoundCue'A_Pickups.Ammo.Cue.A_Pickup_Ammo_Enforcer_Cue'
	MaxDesireability=0.2

	Begin Object Name=AmmoMeshComp
		StaticMesh=StaticMesh'Pickups.Ammo_Enforcer.Mesh.S_Ammo_Enforcer'
		Scale=1.25
		Translation=(X=0.0,Y=0.0,Z=-15.0)
	End Object

	Begin Object Name=CollisionCylinder
		CollisionHeight=14.4
	End Object
}
