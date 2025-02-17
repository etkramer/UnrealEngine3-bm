/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGDizzyMP extends GearPawn_HumanBaseMP
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=348,V=65,UL=57,VL=68)
	HelmetType=class'Item_Helmet_COGDizzyCowboyHat'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGDizzyMP'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGDizzyMP"
	NeedsRevivedGUDSEvent=GUDEvent_BairdNeedsRevived
	WentDownGUDSEvent=GUDEvent_BairdWentDown
	FAS_ChatterNames.Add("COG_Dizzy.FaceFX.Dizzy_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Dizzy.FaceFX.Dizzy_FaceFX_Efforts'

	GoreSkeletalMesh=SkeletalMesh'COG_Dizzy.COG_Dizzy_Gore'
	GorePhysicsAsset=PhysicsAsset'COG_Dizzy.COG_Dizzy_Gore_Physics'
	GoreBreakableJoints=("b_MF_Spine_03","b_MF_Neck","b_MF_Head","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_UpperArm_R","b_MF_Hand_R","b_MF_Calf_L","b_MF_Calf_R")
	HostageHealthBuckets=("b_MF_Hand_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Calf_L","b_MF_Calf_R")
	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Dizzy_Shader_MP'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Dizzy.COG_Dizzy'
		PhysicsAsset=PhysicsAsset'COG_Dizzy.COG_Dizzy_Physics'
	End Object
}
