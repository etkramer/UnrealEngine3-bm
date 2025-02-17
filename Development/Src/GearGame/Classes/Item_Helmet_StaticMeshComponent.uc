/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_Helmet_StaticMeshComponent extends StaticMeshComponent;

defaultproperties
{
	bAllowCullDistanceVolume=FALSE // we don't want helmets affected by culldistance volumes (there is no setter for that value at this time so we do it here)
	MaxDrawDistance=16000
	CollideActors=FALSE
	BlockRigidBody=FALSE
	HiddenGame=TRUE
	AlwaysLoadOnClient=TRUE
	StaticMesh=none // default
	CastShadow=FALSE
	bCastDynamicShadow=FALSE
	MotionBlurScale=0.0f		
	bNotifyRigidBodyCollision=TRUE
	ScriptRigidBodyCollisionThreshold=5.0
	bUseCompartment=FALSE
	RBCollideWithChannels=(Default=TRUE,Pawn=TRUE,DeadPawn=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,FracturedMeshPart=TRUE,SoftBody=TRUE)
	bUseAsOccluder=FALSE
	Scale=1.0f
}
