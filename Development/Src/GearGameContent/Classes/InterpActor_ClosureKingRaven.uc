/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_ClosureKingRaven extends InterpActor_GearBasePlatform
	placeable;

defaultproperties
{
	BlockRigidBody=TRUE
	bCollideActors=TRUE
	bBlockActors=TRUE
	DrawScale=1.5f

	bAlwaysConfineToClampedBase=TRUE
	bDoComplexCameraCollision=FALSE
	bDisallowPawnMovement=true

	Begin Object Name=MyLightEnvironment
		bEnabled=TRUE
	End Object

	//// this obj is designed solely to provide collision for rigid bodies
	//Begin Object Class=StaticMeshComponent Name=RBCollision0
	//End Object
	//Components.Add(RBCollision0)


	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'COG_Chopper.Mesh.cog_kingraven_cine_extended_collision'
	End Object

	//Begin Object Name=RBCollision0
	//	StaticMesh=StaticMesh'AB_Locust_Boat.Meshes.SM_Locust_Busted_Pier01_RBC'

	//	HiddenGame=TRUE

	//	BlockActors=FALSE
	//	BlockZeroExtent=FALSE
	//	BlockNonZeroExtent=FALSE
	//	BlockRigidBody=TRUE
	//End Object
}
