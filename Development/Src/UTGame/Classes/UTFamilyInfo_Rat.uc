/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class UTFamilyInfo_Rat extends UTFamilyInfo
	abstract;

defaultproperties
{
	Faction="RAT"
	FamilyID="RATM"

	CharacterMesh=SkeletalMesh'CH_RATM_Male.Mesh.SK_CH_RATM_Male01'

	ArmMeshPackageName="CH_IronGuard_Arms"
	ArmMeshName="CH_IronGuard_Arms.Mesh.SK_CH_IronGuard_Arms_MaleB_1P"
	ArmSkinPackageName="CH_IronGuard_Arms"
	RedArmMaterialName="CH_IronGuard_Arms.Materials.M_CH_IronG_Arms_FirstPersonArm_VRed"
	BlueArmMaterialName="CH_IronGuard_Arms.Materials.M_CH_IronG_Arms_FirstPersonArm_VBlue"

	CharacterTeamBodyMaterials[0]=MaterialInterface'CH_RATM_Male.Materials.MI_CH_RATM_Male_VRed'
	CharacterTeamHeadMaterials[0]=MaterialInterface'CH_RATM_Male.Materials.MI_CH_RATM_Male_VRed'
	CharacterTeamBodyMaterials[1]=MaterialInterface'CH_RATM_Male.Materials.MI_CH_RATM_Male_VBlue'
	CharacterTeamHeadMaterials[1]=MaterialInterface'CH_RATM_Male.Materials.MI_CH_RATM_Male_VBlue'

	NeckStumpName="SK_CH_IronG_Male_NeckStump01"

	PhysAsset=PhysicsAsset'CH_AnimHuman.Mesh.SK_CH_BaseMale_Physics'
	AnimSets(0)=AnimSet'CH_AnimHuman.Anims.K_AnimHuman_BaseMale'

	BaseMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_IronG_Base'
	BioDeathMICParent=MaterialInstanceConstant'CH_All.Materials.MI_CH_ALL_IronG_BioDeath'

}
