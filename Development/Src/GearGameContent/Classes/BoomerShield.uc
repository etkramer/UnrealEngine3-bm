/**
 * Boomer shield.
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class BoomerShield extends GearShield;

defaultproperties
{
	ShieldExpandSound=SoundCue'Weapon_Shield.Shield.ShieldExpandingCue'
	ShieldRetractSound=SoundCue'Weapon_Shield.Shield.ShieldRetractingCue'
	ShieldStickInGroundSound=SoundCue'Weapon_Shield.Shield.ShieldGroundStickCue'
	ShieldRemoveFromGroundSound=SoundCue'Weapon_Shield.Shield.ShieldGroundRemoveCue'
	ShieldDropSound=SoundCue'Weapon_Shield.Shield.ShieldDropCue'

	Begin Object Name=ShieldMesh0
		SkeletalMesh=SkeletalMesh'Locust_Shield.Meshes.Locust_Shield_Boomer'
		AnimSets(0)=AnimSet'Locust_Shield.Anims.Locust_Shield_Boomer_Animset'
		AnimTreeTemplate=AnimTree'Locust_Shield.Anims.Locust_Shield_Boomer_AnimTree'
		PhysicsAsset=PhysicsAsset'Locust_Shield.Meshes.Locust_Shield_Boomer_Physics'
	End Object

	Begin Object Class=AnimNodeSequence Name=AnimNodeSeq1
	End Object
	Begin Object Name=ShieldMesh1
		SkeletalMesh=SkeletalMesh'Locust_Shield.Meshes.Locust_Shield_Boomer'
		AnimSets(0)=AnimSet'Locust_Shield.Anims.Locust_Shield_Boomer_Animset'
		Animations=AnimNodeSeq1
		PhysicsAsset=PhysicsAsset'Locust_Shield.Meshes.Locust_Shield_Boomer_Physics'
		Translation=(X=0,Y=0,Z=28)
	End Object

	PickupSound=SoundCue'Weapon_Shield.Shield.ShieldPickupCue'
}
