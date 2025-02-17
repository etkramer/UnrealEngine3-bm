/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustGondola extends InterpActor_LocustGondolaBase
	placeable;

/**
 * Note the misnaming, this is actually the barge version that confines the player.  Not worth the LD trouble 
 * that renaming could cause. 
 */

defaultproperties
{
	// rendering staticmesh, has no collision
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'locust_gondola01_sm.SM.Mesh.locust_gondola_NoCollision_SM'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
	End Object

	// this obj is for all collision except rbs
	// has high walls, used everywhere except during mantle sequence
	Begin Object Name=HighWallCollision0
		StaticMesh=StaticMesh'locust_gondola01_sm.SM.Mesh.locust_gondola_collision_highwall'
		HiddenGame=TRUE
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
	End Object

	// this obj is for all collision except rbs
	// has low walls, used during mantle sequence
	Begin Object Name=LowWallCollision0
		StaticMesh=StaticMesh'locust_gondola01_sm.SM.Mesh.locust_gondola01_sm'
		HiddenGame=TRUE
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
	End Object

	// this obj is designed solely to provide collision for rigid bodies
	Begin Object Name=RBCollision0
		StaticMesh=StaticMesh'locust_gondola01_sm.SM.Mesh.locust_gondola_collision_RBC'
		HiddenGame=TRUE
		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
	End Object

	// this obj is for all collision except rbs (@fixme)
	//Begin Object Name=StaticCollision0
	//	StaticMesh=StaticMesh'locust_gondola01_sm.SM.Mesh.locust_gondola01_sm_SOMETHING'
	//End Object

	// this obj is designed solely to provide collision for rigid bodies (@fixme)
	//Begin Object Name=RBCollision0
	//	StaticMesh=StaticMesh''
	//	HiddenGame=TRUE

	//	BlockActors=FALSE
	//	BlockZeroExtent=FALSE
	//	BlockNonZeroExtent=FALSE
	//	BlockRigidBody=TRUE
	//End Object
}
