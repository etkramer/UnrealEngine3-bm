/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearKnockableTree extends GearKnockableActor;

defaultproperties
{
	Begin Object Class=DynamicLightEnvironmentComponent Name=MyLightEnvironment
		bSynthesizeDirectionalLight=False
		bSynthesizeSHLight=False
		bDynamic=FALSE
		MaxShadowResolution=256
	End Object
	Components.Add(MyLightEnvironment)
	LightEnvironment=MyLightEnvironment

	Begin Object Class=SkeletalMeshComponent Name=MeshComp
		SkeletalMesh=SkeletalMesh'GOW_Assault_Trees.SM.Mesh.SK_GOW_Assault_Trees_Breakable_Tree02_leaves'
		PhysicsAsset=PhysicsAsset'GOW_Assault_Trees.SM.Mesh.SK_GOW_Assault_Trees_Breakable_Tree01_bend_Physics'
		WireframeColor=(R=0,G=255,B=128,A=255)
		BlockRigidBody=false
		RBChannel=RBCC_GameplayPhysics
		RBCollideWithChannels=(Default=true,BlockingVolume=TRUE,GameplayPhysics=true,EffectPhysics=true)
		bHasPhysicsAssetInstance=true
		bSkipAllUpdateWhenPhysicsAsleep=true
		CollideActors=true
		BlockActors=true
		BlockZeroExtent=true
		BlockNonZeroExtent=true
		bUpdateKinematicBonesFromAnimation=false
		PhysicsWeight=1.0
		LightEnvironment=MyLightEnvironment
	End Object
	Components.Add(MeshComp)
	Mesh=MeshComp
	CollisionComponent=MeshComp

	Begin Object Name=HitFX
		Template=ParticleSystem'Effects_Level_SP_3.SP_MountKismet.Effects.P_Tree_Hit_Effect'
	End Object

	HitSound=SoundCue'Foley_Crashes.SoundCues.Crash_Wood_Large'
	DelayedHitSound=SoundCue'Ambient_NonLoop.AmbientNonLoop.WoodTreeShakeDrysCue'
}
