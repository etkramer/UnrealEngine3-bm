/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearPawn_LocustAmbient_Bat extends GearPawn
	placeable;


DefaultProperties
{
	Begin Object Name=CollisionCylinder
		CollisionHeight=+0012.000
		CollisionRadius=+0015.000
	End Object
	
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Ambient.Ambient_Cave_Bat'
		PhysicsAsset=PhysicsAsset'Locust_Ambient.Ambient_Cave_Bat_Physics'
		bHasPhysicsAssetInstance=TRUE	// Needed by PhysicsBodyImpact
		AnimSets(0)=AnimSet'Locust_Ambient.Ambient_Cave_Bat_Animset'
	End Object
}