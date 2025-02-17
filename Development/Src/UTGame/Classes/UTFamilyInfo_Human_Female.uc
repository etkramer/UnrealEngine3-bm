/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTFamilyInfo_Human_Female extends UTFamilyInfo_Human
	abstract;

defaultproperties
{
	FamilyID="HUMF"

	//@TODO - placeholder need female mesh
	CharacterMesh=SkeletalMesh'CH_IRNF_Ariel.Mesh.SK_CH_IRNF_Ariel'

	ArmMeshPackageName="CH_IronGuard_Arms"
	ArmMeshName="CH_IronGuard_Arms.Mesh.SK_CH_IronGuard_Arms_MaleB_1P"
	ArmSkinPackageName="CH_IronGuard_Arms"
	RedArmMaterialName="CH_IronGuard_Arms.Materials.M_CH_IronG_Arms_FirstPersonArm_VRed"
	BlueArmMaterialName="CH_IronGuard_Arms.Materials.M_CH_IronG_Arms_FirstPersonArm_VBlue"
																						
	CharacterTeamBodyMaterials[0]=MaterialInterface'CH_IronGuard_Female.Materials.MI_CH_IronG_Fbody02_VRED'
	CharacterTeamHeadMaterials[0]=MaterialInterface'CH_IronGuard_Female.Materials.MI_CH_IronG_FHead02_V01'
	CharacterTeamBodyMaterials[1]=MaterialInterface'CH_IronGuard_Female.Materials.MI_CH_IronG_Fbody02_VBLUE'
	CharacterTeamHeadMaterials[1]=MaterialInterface'CH_IronGuard_Female.Materials.MI_CH_IronG_FHead02_V01'

	NeckStumpName="SK_CH_IronG_Female_NeckStump01"

	PhysAsset=PhysicsAsset'CH_AnimHuman.Mesh.SK_CH_BaseFemale_Physics'
	AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'
	SoundGroupClass=class'UTPawnSoundGroup_HumanFemale'
	VoiceClass=class'UTGame.UTVoice_Lauren'

	BaseMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_IronG_Base'
	BioDeathMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_IronG_BioDeath'

	bIsFemale=true
}
