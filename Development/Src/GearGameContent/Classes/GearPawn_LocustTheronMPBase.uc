
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustTheronMPBase extends GearPawn_LocustBaseMP
	config(Pawn);

/** theron MP helmets don't fly off**/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,7,-2);
}

defaultproperties
{
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustTheron'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustTheronMP"
	FAS_ChatterNames.Add("Locust_Theron_Guard.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Theron_Guard.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Theron_Guard.FaceFX.theron_Guard_FaceFX_Efforts'
	FAS_Efforts(1)=FaceFXAnimSet'Locust_Theron_Guard.FaceFX.theron_Guard_FaceFX_Efforts_Dup'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Locust_Theron_Guard_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
	GoreBreakableJoints=("b_MF_Forearm_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Spine_03","b_MF_Head","b_MF_Face","b_MF_Calf_L","b_MF_Calf_R","b_MF_Foot_R")
	HostageHealthBuckets=("b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Calf_R")
	GoreMorphSets.Add(MorphTargetSet'Locust_Theron_Guard.Theron_Guard_Gore_Morphs')
	MeatShieldMorphTargetName="Meatshield_Morph"

	MPHeadOffset=(X=5,Y=-4,Z=0)

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.LOC_Theron_Shader_MP'

	Begin Object Name=GearPawnMesh
		bEnableFullAnimWeightBodies=true
		MorphSets.Add(MorphTargetSet'Locust_Theron_Guard.Mesh.Theron_Guard_Game_Morphs')
	End Object

	PickupFocusBoneName=b_MF_Weapon_L
	PickupFocusBoneNameKickup=b_MF_Weapon_R	

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0072.000000
	End Object
}