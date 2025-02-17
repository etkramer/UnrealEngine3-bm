/** 
 * Class for Locust Lynx 
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustNinja extends GearPawn_LocustDroneBase
	config(Pawn);

defaultproperties
{
	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Grunt.Mesh.Locust_Grunt_CamSkel'
		PhysicsAsset=PhysicsAsset'Locust_Grunt.PhysicsAsset.Locust_Grunt_CamSkel_Physics'
		Scale3D=(X=0.8,Y=0.8,Z=0.9)
	End Object
}
