
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGRedShirt extends GearPawn_COGGear
	config(Pawn);

/** redshirt helmets don't fly off**/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );

defaultproperties
{
	HelmetType=class'Item_Helmet_COGRandom'

	ControllerClass=class'GearAI_COGGear'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGGear'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'COG_Minh.Minh_Kim_Gore'
	GorePhysicsAsset=PhysicsAsset'COG_Minh.Minh_Kim_Physics'
	GoreBreakableJoints=("b_MF_Hand_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Spine_03","b_MF_Head","b_MF_Face","b_MF_Calf_L","b_MF_Calf_R")
	HostageHealthBuckets=("b_MF_Hand_L","b_MF_Hand_R","b_MF_UpperArm_L","b_MF_UpperArm_R","b_MF_Calf_L","b_MF_Calf_R")

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'COG_Minh.Minh_Kim'
		PhysicsAsset=PhysicsAsset'COG_Minh.Minh_Kim_Physics'
	End Object
}
