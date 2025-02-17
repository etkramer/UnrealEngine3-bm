/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGDomMP extends GearPawn_HumanBaseMP
	config(Pawn);


defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=49,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'
	MPHeadOffset=(X=3,Y=-1,Z=0)

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGDom'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGDomMP"
	NeedsRevivedGUDSEvent=GUDEvent_DomNeedsRevived
	WentDownGUDSEvent=GUDEvent_DomWentDown
	FAS_ChatterNames.Add("COG_Dom.FaceFX.Chatter") 
	FAS_ChatterNames.Add("COG_Dom.FaceFX.Dom_Gears1_Chatter") 	
	FAS_Efforts(0)=FaceFXAnimSet'COG_Dom.FaceFX.Dom_FaceFX_Efforts'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
    GoreSkeletalMesh=SkeletalMesh'COG_Dom.Cog_Dom_Game_Gore_CamSkel'
	GorePhysicsAsset=PhysicsAsset'COG_Dom.COG_Dom_CamSkel_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Calf_R","b_MF_Calf_L","b_MF_Foot_R")
	HostageHealthBuckets=("b_MF_Hand_L","b_MF_UpperArm_R","b_MF_Calf_L","b_MF_Calf_R")


	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Dom_Shader_MP'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Dom.Game.COG_Dom_Game_CamSkel'
		PhysicsAsset=PhysicsAsset'COG_Dom.COG_Dom_CamSkel_Physics'
	End Object
}
