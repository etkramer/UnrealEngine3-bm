/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */
class GearKnockableTreeB extends GearKnockableActor;

simulated function KnockDown(vector HitLocation, vector HitDirection)
{
	// hacked to provide a scripted effect requested by the artists
	GotoState('KnockedDown');
	Mesh.AddImpulse(HitDirection * ImpulseStrength, SkeletalMeshComponent(Mesh).GetBoneLocation('BONE_Section03'), 'BONE_Section03', true);
}

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
		SkeletalMesh=SkeletalMesh'GOW_Assault_Trees.SM.Mesh.SK_GOW_Assault_Trees_Breakable_Tree01_leaves'
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
		Template=ParticleSystem'Effects_Level_SP_1.SP_Assault.P_Tree_Hit_Effect'
	End Object

	HitSound=SoundCue'Vehicle_Derrick.Derrick.Derrick_TreeHit_Cue'
	DelayedHitSound=SoundCue'Vehicle_Derrick.Derrick.Derrick_TreeImpact_Cue'
	bAffectedByWeapons=false
}
