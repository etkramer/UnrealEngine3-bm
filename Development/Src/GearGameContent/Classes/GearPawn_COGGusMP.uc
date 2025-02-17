/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGGusMP extends GearPawn_HumanBaseMP
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=147,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGGus'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGGusMP"
	NeedsRevivedGUDSEvent=GUDEvent_GusNeedsRevived
	WentDownGUDSEvent=GUDEvent_GusWentDown
	FAS_ChatterNames.Add("COG_Gus.FaceFX.Gus_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Gus.FaceFX.Gus_FaceFX_Efforts'

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Gus_Shader_MP'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'COG_Gus.cog_gus_gore'
	GorePhysicsAsset=PhysicsAsset'COG_Gus.COG_Gus_Physics'
	GoreBreakableJoints=("b_MF_Hand_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Spine_03","b_MF_Head","b_MF_Face","b_MF_Calf_L","b_MF_Calf_R","b_MF_Foot_R")
	JointsWithDependantBreaks=((ParentBone="b_MF_Spine_03",DependantBones=("b_MF_UpperArm_R")))


	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Gus.COG_Gus'
		PhysicsAsset=PhysicsAsset'COG_Gus.COG_Gus_Physics'
	End Object
}
