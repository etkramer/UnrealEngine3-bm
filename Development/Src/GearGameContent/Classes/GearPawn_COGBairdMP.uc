/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGBairdMP extends GearPawn_HumanBaseMP
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=0,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGBaird'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGBairdMP"
	NeedsRevivedGUDSEvent=GUDEvent_BairdNeedsRevived
	WentDownGUDSEvent=GUDEvent_BairdWentDown
	FAS_ChatterNames.Add("COG_Baird.FaceFX.Baird_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Baird.FaceFX.Baird_FaceFX_Efforts'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'COG_Baird.cog_baird_gore'
	GorePhysicsAsset=PhysicsAsset'COG_Baird.COG_Baird_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")
	HostageHealthBuckets=("b_MF_Hand_R","b_MF_Hand_L","b_MF_UpperArm_R","b_MF_Calf_L","b_MF_Calf_R")
	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Baird_Shader_MP'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Baird.COG_Baird'
		PhysicsAsset=PhysicsAsset'COG_Baird.COG_Baird_Physics'
	End Object
}