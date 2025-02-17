/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTGibStaticMeshComponent extends StaticMeshComponent;

defaultproperties
{
	MaxDrawDistance=8000
	BlockActors=FALSE
	CollideActors=TRUE
	BlockRigidBody=TRUE
	CastShadow=FALSE
	bCastDynamicShadow=FALSE
	bNotifyRigidBodyCollision=TRUE
	ScriptRigidBodyCollisionThreshold=5.0
	bUseCompartment=FALSE
	RBCollideWithChannels=(Default=TRUE,BlockingVolume=TRUE,Pawn=TRUE,Vehicle=TRUE,GameplayPhysics=TRUE,EffectPhysics=TRUE)
	bUseAsOccluder=FALSE
	Scale=1.0
}
