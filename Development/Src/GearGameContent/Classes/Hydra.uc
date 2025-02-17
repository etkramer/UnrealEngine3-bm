/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class Hydra extends Hydra_Base
	placeable;

defaultproperties
{
	Begin Object Name=SkeletalMeshComponent0
		//SkeletalMesh=SkeletalMesh'Locust_Uber_Reaver.Uber_Reaver'
		SkeletalMesh=SkeletalMesh'Locust_Uber_Reaver.UberReaver_Gore_Soft'
		PhysicsAsset=PhysicsAsset'Locust_Uber_Reaver.Uber_Reaver_Physics'
		AnimTreeTemplate=AnimTree'Locust_Uber_Reaver_Anim.AT_Uber_Reaver'
		AnimSets(0)=AnimSet'Locust_Uber_Reaver.Locust_Uber_Reaver_Anims'
		bAcceptsDynamicDecals=FALSE
		bHasPhysicsAssetInstance=TRUE
	End Object

	Begin Object Name=SkeletalMeshComponent1
		SkeletalMesh=SkeletalMesh'Locust_Skorge.Mesh.Locust_Skorge'
		AnimSets.Add(AnimSet'Locust_Skorge.Anims.Locust_Skorge_Riding_Hydra_Animset')
		AnimTreeTemplate=AnimTree'Locust_Skorge.Locust_Skorge_Riding_Hydra_AnimTree'
	End Object

	Begin Object Name=PSC_TargetLaser0
		Template=ParticleSystem'Locust_Uber_Reaver_Anim.Effects.P_FX_Hydra_Target_Laser'
	End Object

	Begin Object Name=PSC_TargetLaser1
		Template=ParticleSystem'Locust_Uber_Reaver_Anim.Effects.P_FX_Hydra_Target_Laser'
	End Object

	Begin Object Name=PSC_Spittle0
		Template=ParticleSystem'Locust_Uber_Reaver.Effects.P_Uber_Reaver_Mouth_Roar'
	End Object

	SupportedEvents.Add(class'GearGame.SeqEvt_HydraPartDamaged')

	PartShootBody[0]="b_UR_Jaw"
	PartShootBody[1]="b_UR_Turret"
	PartShootBody[2]="b_UR_Tail_01"
	PartShootBody[3]="b_UR_Tenticle_Front_L_01" // FL
	PartShootBody[4]="b_UR_Tenticle_Back_L_01" // BL
	PartShootBody[5]="b_UR_Tenticle_Front_R_01" // FR
	PartShootBody[6]="b_UR_Tenticle_Back_R_01" // BR

	HideClawBoneNames[0]="b_UR_Tenticle_Front_L_08"
	HideClawBoneNames[1]="b_UR_Tenticle_Back_L_08"
	HideClawBoneNames[2]="b_UR_Tenticle_Front_R_08"
	HideClawBoneNames[3]="b_UR_Tenticle_Back_R_08"

	GoreBreakableJoints=("b_UR_Tenticle_Front_L_07","b_UR_Tenticle_Back_L_05","b_UR_Tenticle_Front_R_07","b_UR_Tenticle_Back_R_05")

	HelmetGoreBoneName(0)=gore_helmet_r
	HelmetGoreBoneName(1)=gore_helmet_l
	MouthGoreBoneName(0)=gore_headchunk1
	MouthGoreBoneName(1)=gore_headchunk02
	MouthGoreBoneName(2)=gore_headchunk03
	ButtGoreBoneName(0)=gore_back_skin01
	ButtGoreBoneName(1)=gore_back_skin02
	
	LaserNoLockColor=(Z=1.0)
	LaserLockColor=(X=1.0)
	LaserLength=3000.0

	// FL, BL, FR, BR (like enum)

	// FL
	GrabActorOffset[0]=(X=-1100,Y=-250,Z=-300)
	GrabActorRotOffset[0]=(Pitch=3640,Roll=0,Yaw=3640)

	// BL
	GrabActorOffset[1]=(X=-1200,Y=-250,Z=-30)
	GrabActorRotOffset[1]=(Pitch=0,Roll=0,Yaw=3640)

	// FR
	GrabActorOffset[2]=(X=-1150,Y=100,Z=-300)
	GrabActorRotOffset[2]=(Pitch=3640,Roll=-14500,Yaw=-910)

	// BR
	GrabActorOffset[3]=(X=-1200,Y=100,Z=-400)
	GrabActorRotOffset[3]=(Pitch=3640,Roll=-14500,Yaw=-910)


	GrabIKBlendTime=0.4
	GrabIKDelay=0.8
	GrabSBBlendTime=0.45
	GrabSBDelay=0.65
	GrabSBStrength=0.9

	BlendTentacleDelay=1.5
	BlendTentacleTime=0.6
	TrailTentacleDelay=1.5
	TrailBlendTime=0.6

	FireOffsetZ=100.0

	HydraGravZScale=2.0
	ClawHideTime=5.0

	SuddenAccelSound=SoundCue'Locust_Hydra_Efforts.Hydra.Hydra_LungeForwardCue'
	AmbientVocalSound=SoundCue'Locust_Hydra_Efforts.Hydra.Hydra_VocalAmbientCue'
	PainVocalSound=SoundCue'Locust_Hydra_Efforts.Hydra.Hydra_VocalPainCue'

	// Flying sound
	Begin Object Class=AudioComponent Name=AudioComponent0
		SoundCue=SoundCue'Locust_Hydra_Efforts.Hydra.Hydra_FlyLoop01Cue'	
		bAutoPlay=TRUE
		bStopWhenOwnerDestroyed=TRUE
		bShouldRemainActiveIfDropped=TRUE
	End Object
	FlyingLoopSound=AudioComponent0
	Components.Add(AudioComponent0)
}
