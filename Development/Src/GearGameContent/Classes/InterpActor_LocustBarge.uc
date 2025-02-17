/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustBarge extends InterpActor_LocustBargeBase
	placeable;

/**
 * Note the misnaming, this is actually the barge version that confines the player.  Not worth the LD trouble 
 * that renaming could cause. 
 */

defaultproperties
{
	// this one is the rendering mesh, plus highwall collision sans turret and crates
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_01_NoTurret_SM'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
	End Object

	// (this one has the collision for the turret and crates)
	Begin Object Class=StaticMeshComponent Name=TurretCreatesCollision1
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_TurretCrates_Collision'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
		HiddenGame=TRUE
	End Object
	Components.Add(TurretCreatesCollision1)

	// placeholder mesh with collision for rigid bodies
	Begin Object Name=RBCollision0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_RBC'

		HiddenGame=TRUE

		BlockActors=FALSE
		BlockZeroExtent=FALSE
		BlockNonZeroExtent=FALSE
		BlockRigidBody=TRUE
	End Object
}