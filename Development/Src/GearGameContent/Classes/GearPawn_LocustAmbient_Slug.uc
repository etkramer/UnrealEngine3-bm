/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/
class GearPawn_LocustAmbient_Slug extends GearPawn
	placeable;


DefaultProperties
{
	Begin Object Name=CollisionCylinder
		CollisionHeight=+0012.000
		CollisionRadius=+0015.000
	End Object

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Ambient.Anims.Ambient_Cave_Slug'
		PhysicsAsset=PhysicsAsset'Locust_Ambient.Ambient_Cave_Slug_Physics'
		bHasPhysicsAssetInstance=TRUE	// Needed by PhysicsBodyImpact
		AnimSets(0)=AnimSet'Locust_Ambient.Ambient_Cave_Slug_Animset'
	End Object
}