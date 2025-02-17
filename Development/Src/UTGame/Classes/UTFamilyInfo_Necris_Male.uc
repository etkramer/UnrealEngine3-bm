/**
* Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
*/

class UTFamilyInfo_Necris_Male extends UTFamilyInfo_Necris
	abstract;

defaultproperties
{
	FamilyID="NECM"

	//@TODO - placeholder need male mesh
	CharacterMesh=SkeletalMesh'CH_NECM_Damian.Mesh.SK_CH_NECM_Damian'

	CharacterTeamBodyMaterials[0]=MaterialInterface'CH_Necris_Male1.Materials.MI_CH_Necris_MBody01_VRed'
	CharacterTeamHeadMaterials[0]=MaterialInterface'CH_Necris_Male1.Materials.MI_CH_Necris_MHead01_VRed'
	CharacterTeamBodyMaterials[1]=MaterialInterface'CH_Necris_Male1.Materials.MI_CH_Necris_MBody01_VBlue'
	CharacterTeamHeadMaterials[1]=MaterialInterface'CH_Necris_Male1.Materials.MI_CH_Necris_MHead01_VBlue'

	PhysAsset=PhysicsAsset'CH_AnimHuman.Mesh.SK_CH_BaseMale_Physics'
	AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'

	NeckStumpName="SK_CH_Necris_Male_NeckStump01"

	BaseMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_All_Necris_Base'
	BioDeathMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_Necris_BioDeath'

	VoiceClass=class'UTVoice_NecrisMale'
	
	DefaultMeshScale=1.025
	BaseTranslationOffset=6.0
}