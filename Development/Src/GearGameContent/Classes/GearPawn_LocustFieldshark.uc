/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 **/
class GearPawn_LocustFieldshark extends GearPawn;

defaultproperties
{
	//@fixme - convert to skeletal mesh when the asset is available
	Begin Object Class=StaticMeshComponent Name=GearPawnMesh2
	    StaticMesh=StaticMesh'Locust_Nemaslug.locust_nemaslug_mesh'
		BlockZeroExtent=true
		CollideActors=true
		BlockRigidBody=true
	End Object
	CollisionComponent=GearPawnMesh2
	Components.Add(GearPawnMesh2)

	Health=25
}
