/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTFamilyInfo_Human_Male extends UTFamilyInfo_Human
	abstract;

defaultproperties
{
	FamilyID="HUMM"

	CharacterMeshName="CH_IronGuard_Male.Mesh.SK_CH_IronGuard_MaleA"
	CharacterMesh=SkeletalMesh'CH_IronGuard_Male.Mesh.SK_CH_IronGuard_MaleA'

	ArmMeshPackageName="CH_IronGuard_Arms"
	ArmMeshName="CH_IronGuard_Arms.Mesh.SK_CH_IronGuard_Arms_MaleB_1P"
	ArmSkinPackageName="CH_IronGuard_Arms"
	RedArmMaterialName="CH_IronGuard_Arms.Materials.M_CH_IronG_Arms_FirstPersonArm_VRed"
	BlueArmMaterialName="CH_IronGuard_Arms.Materials.M_CH_IronG_Arms_FirstPersonArm_VBlue"

	CharacterTeamBodyMaterials[0]=MaterialInterface'CH_IronGuard_Male.Materials.MI_CH_IronG_Mbody01_VRed'
	CharacterTeamHeadMaterials[0]=MaterialInterface'CH_IronGuard_Male.Materials.MI_CH_IronG_MHead01_VRed'
	CharacterTeamBodyMaterials[1]=MaterialInterface'CH_IronGuard_Male.Materials.MI_CH_IronG_Mbody01_VBlue'
	CharacterTeamHeadMaterials[1]=MaterialInterface'CH_IronGuard_Male.Materials.MI_CH_IronG_MHead01_VBlue'

	NeckStumpName="SK_CH_IronG_Male_NeckStump01"

	PhysAsset=PhysicsAsset'CH_AnimHuman.Mesh.SK_CH_BaseMale_Physics'
	AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'

	BaseMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_IronG_Base'
	BioDeathMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_IronG_BioDeath'
}
