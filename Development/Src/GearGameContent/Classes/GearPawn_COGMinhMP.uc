
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGMinhMP extends GearPawn_HumanBaseMP
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=98,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGMinh'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGMinhMP"
	NeedsRevivedGUDSEvent=GUDEvent_MinhNeedsRevived
	WentDownGUDSEvent=GUDEvent_MinhWentDown
	FAS_ChatterNames.Add("COG_Minh.FaceFX.Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Minh.FaceFX.COG_Minh_FaceFX_Efforts'

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Minh_Shader_MP'

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
