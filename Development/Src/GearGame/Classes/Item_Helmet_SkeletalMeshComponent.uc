/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Item_Helmet_SkeletalMeshComponent extends SkeletalMeshComponent;


defaultproperties
{
	MaxDrawDistance=4000
	BlockActors=FALSE
	CollideActors=FALSE
	BlockRigidBody=TRUE
	CastShadow=FALSE
	bCastDynamicShadow=FALSE
	bNotifyRigidBodyCollision=TRUE
	ScriptRigidBodyCollisionThreshold=5.0
	bUseCompartment=FALSE
	RBCollideWithChannels=(Default=TRUE,Pawn=TRUE,DeadPawn=TRUE,BlockingVolume=TRUE,EffectPhysics=TRUE,FracturedMeshPart=TRUE,SoftBody=TRUE)
	bUseAsOccluder=FALSE
	bUpdateSkelWhenNotRendered=FALSE
	bHasPhysicsAssetInstance=FALSE
	PhysicsWeight=0.0f

	Scale=1.0f
	//bSkipAllUpdateWhenPhysicsAsleep=TRUE

	MotionBlurScale=0.0f
	bAcceptsDynamicDecals=FALSE // Each decal on them causes entire SkelMesh to be rerendered
	bEnableFullAnimWeightBodies=TRUE // allows flappies to move
	bUpdateKinematicBonesFromAnimation= TRUE // allows flappies to move

	AlwaysLoadOnClient=TRUE
	HiddenGame=TRUE
	MinDistFactorForKinematicUpdate=0.2f
}

