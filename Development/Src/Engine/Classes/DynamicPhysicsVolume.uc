/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

/**
 * This is a movable physics volume. It can be moved by matinee, being based on
 * dynamic objects, etc.
 */
class DynamicPhysicsVolume extends PhysicsVolume
	showcategories(Movement)
	placeable;

defaultproperties
{
	Physics=PHYS_Interpolating
	bStatic=false

	bAlwaysRelevant=true
	bReplicateMovement=true
	bOnlyDirtyReplication=true
	RemoteRole=ROLE_None

	bColored=true
	BrushColor=(R=100,G=255,B=255,A=255)
}
