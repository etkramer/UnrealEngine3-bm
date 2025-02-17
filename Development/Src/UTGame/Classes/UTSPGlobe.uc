/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTSPGlobe extends SkeletalMeshActor;

var name GlobeTag;

simulated function PostBeginPlay()
{
	Super.PostBeginPlay();
	RotationRate.Yaw = -1024;
	SetPhysics(PHYS_Rotating);
}

defaultproperties
{
}
