/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGHoffmanMP extends GearPawn_HumanBaseMP
	config(Pawn);


defaultproperties
{
	HelmetType=class'Item_Helmet_None'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=0,V=0,UL=48,VL=63)
	MPHeadOffset=(X=5,Y=-2,Z=0)

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGHoffman'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGHoffmanMP"
	NeedsRevivedGUDSEvent=GUDEvent_HoffmanNeedsRevived
	WentDownGUDSEvent=GUDEvent_HoffmanWentDown
	FAS_ChatterNames.Add("COG_Hoffman.FaceFX.COG_Hoffman_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Hoffman.FaceFX.COG_Hoffman_FaceFX_Efforts'

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Hoffman_Shader_MP'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'COG_Hoffman.COG_Hoffman_Gore'
	GorePhysicsAsset=PhysicsAsset'COG_Hoffman.COG_Hoffman_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L","b_MF_Foot_L")
	HostageHealthBuckets=("b_MF_Hand_R","b_MF_UpperArm_L","b_MF_Foot_L","b_MF_Calf_R")

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'COG_Hoffman.COG_Hoffman'
		PhysicsAsset=PhysicsAsset'COG_Hoffman.COG_Hoffman_Physics' // tmp just use marcus's for now
	End Object
}
