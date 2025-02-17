
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustHunterArmorMP extends GearPawn_LocustHunterBaseMP
	config(Pawn);

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,4,-2);
}

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=63,V=64,UL=62,VL=62)

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.LOC_Hunter_Shader_MP'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Hunter.Locust_Hunter_Armor_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Hunter.Locust_Hunter_Armor_NoGrenades_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L","b_MF_Foot_L")
	GoreMorphSets.Add(MorphTargetSet'Locust_Hunter.Locust_Hunter_Armor_Gore_Morphs')

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'Locust_Hunter.Locust_Hunter_Armor_NoGrenades'
	    PhysicsAsset=PhysicsAsset'Locust_Hunter.Locust_Hunter_Armor_NoGrenades_Physics'
		MorphSets.Add(MorphTargetSet'Locust_Hunter.Locust_Hunter_Armor_Game_Morphs')
	End Object
}