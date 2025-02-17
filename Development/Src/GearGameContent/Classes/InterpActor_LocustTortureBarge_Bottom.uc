/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustTortureBarge_Bottom extends InterpActor_LocustTortureBargeBase
	placeable;

defaultproperties
{
	// player is never on a bottom while it's moving in Gears 2
	// teleporting cause at least one problem, so we're just not treating it as a clamped base
	bClampedBaseEnabled=FALSE

	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_TortureBarge.Mesh.Barge_Base_All'
	End Object
}