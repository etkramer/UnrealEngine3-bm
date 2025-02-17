
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustGeneralRaamMP extends GearPawn_LocustTheronMP
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=126,V=0,UL=62,VL=62)
	HelmetType=class'Item_Helmet_None'
	MPHeadOffset=(X=8,Y=-2,Z=0)

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustRaam'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustRaamMP"
	FAS_ChatterNames.Empty()
	FAS_ChatterNames.Add("Locust_General_Raam.FaceFX.Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_General_Raam.FaceFX.Gen_Raam_FaceFX_Efforts'

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.LOC_Raam_Shader_MP'

	GoreSkeletalMesh=SkeletalMesh'Locust_General_Raam.Locust_General_Raam_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
	GoreBreakableJoints=("b_MF_Calf_L","b_MF_Calf_R","b_MF_Foot_R","b_MF_Spine_03","b_MF_UpperArm_L","b_MF_Forearm_L","b_MF_UpperArm_R","b_MF_Forearm_R","b_MF_Head","b_MF_Face")
	HostageHealthBuckets=("b_MF_Forearm_L","b_MF_Forearm_R","b_MF_Foot_R","b_MF_UpperArm_R","b_MF_Calf_R","b_MF_Calf_L")
	GoreMorphSets.Empty
	GoreMorphSets.Add(MorphTargetSet'Locust_General_Raam.General_Raam_Gore_Morphs')
	MeatShieldMorphTargetName="Meatshield_Morph"

	PhysHRMotorStrength=(X=750,Y=0)

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_General_Raam.Locust_General_Raam'
		PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
		MorphSets.Empty
		MorphSets.Add(MorphTargetSet'Locust_General_Raam.General_Raam_Game_Morphs')
	End Object
}