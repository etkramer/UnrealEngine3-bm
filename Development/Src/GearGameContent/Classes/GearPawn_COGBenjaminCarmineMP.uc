/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGBenjaminCarmineMP extends GearPawn_HumanBaseMP
	config(Pawn);

/** redshirt helmets don't fly off**/
simulated function RemoveAndSpawnAHelmet( Vector ApplyImpulse, class<DamageType> DamageType, bool bForced );

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=294,V=0,UL=53,VL=63)
	HelmetType=class'Item_Helmet_COGTwo'
	MPHeadOffset=(X=5,Y=-2,Z=0)

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGCarmine'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGCarmineMP"
	NeedsRevivedGUDSEvent=GUDEvent_CarmineNeedsRevived
	WentDownGUDSEvent=GUDEvent_CarmineWentDown

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