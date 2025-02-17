/**
*	Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class Gear_GoreSkelActor_Brumak extends Gear_SkelMeshActor_Gore
	placeable;

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'Locust_Brumak.Gore.Brumak_Gore'
		AnimTreeTemplate=AnimTree'Locust_Brumak.Brumak_Anim_Tree_Blend'
		PhysicsAsset=PhysicsAsset'Locust_Brumak.BrumakMesh_Physics'
	End Object

	NoGoreDamageTypes.Add(class'GDT_AssaultRifle')
	NoGoreDamageTypes.Add(class'GDT_LocustAssaultRifle')
	NoGoreDamageTypes.Add(class'GDT_LocustBurstPistol')
	NoGoreDamageTypes.Add(class'GDT_LocustPistol')
	NoGoreDamageTypes.Add(class'GDT_Melee')
	NoGoreDamageTypes.Add(class'GDT_Shotgun')
	NoGoreDamageTypes.Add(class'GDT_SniperRifle')
}