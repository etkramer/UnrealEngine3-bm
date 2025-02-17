/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustBarge_NoTurrets extends InterpActor_LocustBarge
	placeable;

/**
 * This is the barge used in the Leviathan fight.  It has no turrets on it, nor does
 * it have crates.  Also, the player can climb into the driver's area, so those walls
 * should have low collision.
 */

var protected StaticMeshComponent FrontRowCrateCollision;
var protected StaticMeshComponent MidRowCrateCollision;
var protected StaticMeshComponent RearRowCrateCollision;

var repnotify bool bFrontCollisionDisabled, bMidCollisionDisabled, bRearCollisionDisabled;

replication
{
	if (bNetDirty)
		bFrontCollisionDisabled, bMidCollisionDisabled, bRearCollisionDisabled;
}


function OnLeviathanBargeControl(SeqAct_LeviathanBargeControl Action)
{
	if (Action.InputLinks[2].bHasImpulse)
	{
		// disable front row crate collision
		DetachComponent(FrontRowCrateCollision);
		bFrontCollisionDisabled = true;
	}
	else if (Action.InputLinks[1].bHasImpulse)
	{
		// disable middle row crate collision
		DetachComponent(MidRowCrateCollision);
		bMidCollisionDisabled = true;
	}
	else
	{
		// disable rear row crate collision
		DetachComponent(RearRowCrateCollision);
		bRearCollisionDisabled = true;
	}
}

simulated event ReplicatedEvent(name VarName)
{
	if (VarName == nameof(bFrontCollisionDisabled))
	{
		if (bFrontCollisionDisabled)
		{
			DetachComponent(FrontRowCrateCollision);
		}
	}
	else if (VarName == nameof(bMidCollisionDisabled))
	{
		if (bMidCollisionDisabled)
		{
			DetachComponent(MidRowCrateCollision);
		}
	}
	else if (VarName == nameof(bRearCollisionDisabled))
	{
		if (bRearCollisionDisabled)
		{
			DetachComponent(RearRowCrateCollision);
		}
	}
	else
	{
		Super.ReplicatedEvent(VarName);
	}
}

defaultproperties
{
	Components.Remove(TurretCreatesCollision1)

	Begin Object Class=StaticMeshComponent Name=FrontCrateCollision0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_Crates01_Collision'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
		HiddenGame=TRUE
	End Object
	Components.Add(FrontCrateCollision0)
	FrontRowCrateCollision=FrontCrateCollision0

	Begin Object Class=StaticMeshComponent Name=MidCrateCollision0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_Crates02_Collision'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
		HiddenGame=TRUE
	End Object
	Components.Add(MidCrateCollision0)
	MidRowCrateCollision=MidCrateCollision0

	Begin Object Class=StaticMeshComponent Name=RearCrateCollision0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_Crates03_Collision'
		bDisableAllRigidBody=TRUE		// all rb collision will happen with the RB Collision mesh
		BlockRigidBody=FALSE
		HiddenGame=TRUE
	End Object
	Components.Add(RearCrateCollision0)
	RearRowCrateCollision=RearCrateCollision0

	Begin Object Name=RBCollision0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_NoTurret_RBC'
	End Object
}
