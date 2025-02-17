/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class UTFamilyInfo_Necris_Female extends UTFamilyInfo_Necris
	abstract;

defaultproperties
{
	FamilyID="NECF"

	CharacterMesh=SkeletalMesh'CH_NECF_Akasha.Mesh.SK_CH_NECF_Akasha'

	CharacterTeamBodyMaterials[0]=MaterialInterface'CH_Necris_Female.Materials.MI_CH_Necris_FBody01_VRed'
	CharacterTeamHeadMaterials[0]=MaterialInterface'CH_Necris_Female.Materials.MI_CH_Necris_FHead01_VRed'
	CharacterTeamBodyMaterials[1]=MaterialInterface'CH_Necris_Female.Materials.MI_CH_Necris_FBody01_VBlue'
	CharacterTeamHeadMaterials[1]=MaterialInterface'CH_Necris_Female.Materials.MI_CH_Necris_FHead01_VBlue'

	PhysAsset=PhysicsAsset'CH_AnimHuman.Mesh.SK_CH_BaseFemale_Physics'
	AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'
	SoundGroupClass=class'UTPawnSoundGroup_HumanFemale'
	VoiceClass=class'UTGame.UTVoice_Akasha'

	NeckStumpName="SK_CH_Necris_Female_NeckStump01"

	BaseMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_All_Necris_Base'
	BioDeathMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_Necris_BioDeath'

	bIsFemale=true
}