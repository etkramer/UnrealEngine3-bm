/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class SkelMeshActor_Unfriendly extends SkeletalMeshActorMAT
	abstract;

/** Allows subclasses to have specific areas make crosshair go red. */
simulated function bool HitAreaIsUnfriendly(TraceHitInfo HitInfo)
{
	return FALSE;
}