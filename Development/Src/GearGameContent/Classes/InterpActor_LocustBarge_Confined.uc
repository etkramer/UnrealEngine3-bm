/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustBarge_Confined extends InterpActor_LocustBarge
	placeable;

/**
 * Note, this is misnamed, but not worth the LD trouble that renaming could cause. 
 * This class is actually not the one that confines the player, so it has the old low collision.
 * This is used for the barge that crashes and the player walks across.  The player is never on this
 * while it moves.
 */

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_Barge.Meshes.Locust_Barge_SM'
	End Object
}