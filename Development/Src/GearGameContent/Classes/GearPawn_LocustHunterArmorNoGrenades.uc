
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustHunterArmorNoGrenades extends GearPawn_LocustHunterBase
	config(Pawn);

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,4,-2);
}

defaultproperties
{
	DefaultInventory(0)=class'GearGame.GearWeap_ShotGun'
	DefaultInventory(1)=class'GearGame.GearWeap_FragGrenade'
	DefaultInventory(2)=class'GearGame.GearWeap_LocustPistol'

	ShoulderPadLeftType=class'Item_ShoulderPad_LocustHunterSpikes'

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


