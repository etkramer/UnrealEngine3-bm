
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGTaiMP extends GearPawn_HumanBaseMP
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=407,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGTai'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGTaiMP"
	NeedsRevivedGUDSEvent=GUDEvent_BairdNeedsRevived
	WentDownGUDSEvent=GUDEvent_BairdWentDown
	FAS_ChatterNames.Add("COG_Tai.FaceFX.Tai_FaceFX_Chatter") 
	FAS_Efforts(0)=FaceFXAnimSet'COG_Tai.FaceFX.Tai_FaceFX_Efforts'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'COG_Tai.COG_Tai_Gore_MP'
	GorePhysicsAsset=PhysicsAsset'COG_Tai.COG_Tai_Physics'
	GoreBreakableJoints=("b_MF_Head","b_MF_Face","b_MF_Calf_R","b_MF_Foot_L","b_MF_Spine_03","b_MF_UpperArm_L","b_MF_UpperArm_R","b_MF_Hand_L","b_MF_Hand_R")
	HostageHealthBuckets=("b_MF_Foot_L","b_MF_Calf_R","b_MF_Hand_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R")

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Thai_Shader_MP'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Tai.COG_Tai_MP'
		PhysicsAsset=PhysicsAsset'COG_Tai.COG_Tai_Physics'
	End Object
}