/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGMarcusMP extends GearPawn_HumanBaseMP
	config(Pawn);


defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=196,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGMarcus'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGMarcusMP"
	NeedsRevivedGUDSEvent=GUDEvent_MarcusNeedsRevived
	WentDownGUDSEvent=GUDEvent_MarcusWentDown
	FAS_ChatterNames.Add("COG_MarcusFenix.FaceFX.marcus_FaceFX_RadioChatter") // ??? do we need this?
	FAS_ChatterNames.Add("COG_MarcusFenix.marcus_FaceFX_Chatter") 
	FAS_Efforts(0)=FaceFXAnimSet'COG_MarcusFenix.FaceFX.Marcus_FaceFX_Efforts'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'COG_MarcusFenix.COG_Marcus_Game_Gore_CamSkel'
	GorePhysicsAsset=PhysicsAsset'COG_MarcusFenix.COG_Marcus_Fenix_CamSkel_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")

	RagdollLimitScaleTable(8)=(RB_ConstraintName="b_MF_Head",Swing1Scale=0.5,Swing2Scale=0.5,TwistScale=0.5)

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.COG_Marcus_Shader_MP'

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_MarcusFenix.COG_Marcus_Fenix_CamSkel'
		PhysicsAsset=PhysicsAsset'COG_MarcusFenix.COG_Marcus_Fenix_CamSkel_Physics'
	End Object
}
