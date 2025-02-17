/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_Raft extends InterpActor_RaftBase
	placeable;

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'AB_Locust_Boat.Meshes.SM_Locust_Busted_Pier01'
	End Object

	Begin Object Name=EntryWayCollision0
		StaticMesh=StaticMesh'AB_Locust_Boat.Meshes.SM_Locust_Busted_Pier01_Closed_Collision'
		HiddenGame=TRUE
	End Object

	Begin Object Name=RBCollision0
		StaticMesh=StaticMesh'AB_Locust_Boat.Meshes.SM_Locust_Busted_Pier01_RBC'

		HiddenGame=TRUE

		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
	End Object
}