/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTFamilyInfo_Skaarj extends UTFamilyInfo
	abstract;

defaultproperties
{
	Faction="Skaarj"
	FamilyID="SKJM"

	CharacterMesh=SkeletalMesh'CH_SKJM_Szalor.SK_CH_SKJM_Szalor'							   

	//@TODO - all placeholder from KRALL, replace w/SKAARJ
	ArmMeshPackageName="CH_Krall_Arms"
	ArmMeshName="CH_Krall_Arms.Mesh.SK_CH_Krall_Arms_MaleA_1P"
	ArmSkinPackageName="CH_Krall_Arms"
	RedArmMaterialName="CH_Krall_Arms.Materials.MI_CH_Krall_Arms_MFirstPersonArm_VRed"
	BlueArmMaterialName="CH_Krall_Arms.Materials.MI_CH_Krall_Arms_MFirstPersonArm_VBlue"

	CharacterTeamBodyMaterials[0]=MaterialInterface'CH_SKJM_Szalor.MI_CH_SKJM_Szalor_BODY_VRED'
	CharacterTeamHeadMaterials[0]=MaterialInterface'CH_SKJM_Szalor.MI_CH_SKJM_Szalor_HEAD_VRED'
	CharacterTeamBodyMaterials[1]=MaterialInterface'CH_SKJM_Szalor.MI_CH_SKJM_Szalor_BODY_VBLUE'
	CharacterTeamHeadMaterials[1]=MaterialInterface'CH_SKJM_Szalor.MI_CH_SKJM_Szalor_HEAD_VBLUE'

	PhysAsset=PhysicsAsset'CH_AnimKrall.Mesh.SK_CH_AnimKrall_Male01_Physics'
	AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'
	AnimSets(1)=AnimSet'CH_AnimKrall.Anims.K_AnimKrall_Base'
	LeftFootBone=b_LeftFoot
	RightFootBone=b_RightFoot
	TakeHitPhysicsFixedBones[0]=b_LeftFoot
	TakeHitPhysicsFixedBones[1]=b_RightFoot

	BaseMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_Krall_Base'
	BioDeathMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_Krall_BioDeath'

	DeathMeshSkelMesh=SkeletalMesh'CH_Skeletons.Mesh.SK_CH_Skeleton_Krall_Male'
	SkeletonBurnOutMaterials=(MaterialInstanceTimeVarying'CH_Skeletons.Materials.MITV_CH_Skeletons_Krall_01_BO',MaterialInstanceTimeVarying'CH_Skeletons.Materials.MITV_CH_Skeletons_Krall_01_BO')

	DeathMeshNumMaterialsToSetResident=2

	//DeathMeshBreakableJoints=("b_LeftArm","b_RightArm","b_LeftLegUpper","b_RightLegUpper")

	NonTeamEmissiveColor=(R=0.0,G=0.0,B=0.0)
	NonTeamTintColor=(R=4.0,G=3.0,B=2.0)

	HeadGib=(BoneName=b_Head,GibClass=class'UTGib_KrallHead',bHighDetailOnly=false)
	SoundGroupClass=class'UTPawnSoundGroup_Krall'
	VoiceClass=class'UTVoice_Krall'

	Gibs[0]=(BoneName=b_LeftForeArm,GibClass=class'UTGib_KrallArm',bHighDetailOnly=false)
	Gibs[1]=(BoneName=b_RightForeArm,GibClass=class'UTGib_KrallHand',bHighDetailOnly=true)
	Gibs[2]=(BoneName=b_LeftLeg,GibClass=class'UTGib_KrallLeg',bHighDetailOnly=false)
	Gibs[3]=(BoneName=b_RightLeg,GibClass=class'UTGib_KrallLeg',bHighDetailOnly=false)
	Gibs[4]=(BoneName=b_Spine,GibClass=class'UTGib_KrallTorso',bHighDetailOnly=false)
	Gibs[5]=(BoneName=b_RightClav,GibClass=class'UTGib_KrallBone',bHighDetailOnly=false)

	DefaultMeshScale=1.0
	DrivingDrawScale=0.85
	BaseTranslationOffset=2.0
	PortraitExtraOffset=(X=35,Z=2)  //adjustment to fit the Krall head in the portrait
}
