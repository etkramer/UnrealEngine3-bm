/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkeletalMeshActor_LeviathanWaterStream extends SkeletalMeshActorMATSpawnable
	placeable;

var SkeletalMeshComponent Mesh;

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		SkeletalMesh=SkeletalMesh'Locust_Leviathan.Mesh.WaterSheet'
		AnimTreeTemplate=AnimTree'Locust_Leviathan.AT_WaterSheet'
	End Object
	Mesh=SkeletalMeshComponent0
	
	bHardAttach=TRUE

	RemoteRole=ROLE_None
}