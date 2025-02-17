/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class InterpActor_LocustTortureBarge_Top extends InterpActor_LocustTortureBargeBase
	placeable;

defaultproperties
{
	Begin Object Name=StaticMeshComponent0
		StaticMesh=StaticMesh'Locust_TortureBarge.Mesh.Barge_Top_All'
	End Object

	// the crates are a separate mesh
	Begin Object Class=StaticMeshComponent Name=TortureBargeCrates0
		StaticMesh=StaticMesh'Locust_TortureBarge.Mesh.Barge_Top_Crates'
	End Object
	Components.Add(TortureBargeCrates0)
}