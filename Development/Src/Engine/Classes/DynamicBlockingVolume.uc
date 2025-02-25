/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This is a movable blocking volume. It can be moved by matinee, being based on
 * dynamic objects, etc.
 */
class DynamicBlockingVolume extends BlockingVolume
	native
	showcategories(Movement)
	placeable;

cpptext
{
	virtual void CheckForErrors();
}

struct CheckpointRecord
{
	var vector Location;
	var rotator Rotation;
	var bool bCollideActors;
	var bool bBlockActors;
	var bool bNeedsReplication;
};

function CreateCheckpointRecord(out CheckpointRecord Record)
{
	Record.Location = Location;
	Record.Rotation = Rotation;
	Record.bCollideActors = bCollideActors;
	Record.bBlockActors = bBlockActors;
	Record.bNeedsReplication = (RemoteRole != ROLE_None);
}

function ApplyCheckpointRecord(const out CheckpointRecord Record)
{
	if (!bHardAttach)
	{
		SetLocation(Record.Location);
		SetRotation(Record.Rotation);
	}
	// always leave component colliding and just use the Actor flags
	CollisionComponent.SetActorCollision(true, true);
	CollisionComponent.SetTraceBlocking(false, true);
	CollisionComponent.SetBlockRigidBody(Record.bCollideActors);
	SetCollision(Record.bCollideActors, Record.bBlockActors);

	if (Record.bNeedsReplication)
	{
		ForceNetRelevant();
	}
}

defaultproperties
{
	//@todo - Change back to PHYS_None
	Physics=PHYS_Interpolating

	bStatic=false

	bAlwaysRelevant=true
	bReplicateMovement=true
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_None

	BrushColor=(R=255,G=255,B=100,A=255)
}
