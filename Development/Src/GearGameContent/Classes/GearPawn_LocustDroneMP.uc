
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustDroneMP extends GearPawn_LocustBaseMP
	config(Pawn);

simulated function vector GetWeaponAimIKPositionFix()
{
	return Super.GetWeaponAimIKPositionFix() + vect(0,4,0);
}

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=190,V=0,UL=62,VL=62)
	HelmetType=class'Item_Helmet_None'

	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustDroneB'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustDroneMP"
	MasterGUDBankClassNames(1)="GearGameContent.GUDData_LocustDroneBMP"
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter")
	FAS_ChatterNames.Add("Locust_Grunt.FaceFX.Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Grunt.FaceFX.Drone_Efforts'

	PhysHRMotorStrength=(X=200,Y=0)

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.LOC_Grunt_Shader_MP'


	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Grunt.Mesh.Locust_Grunt_Gore_SoftWeights'
	GorePhysicsAsset=PhysicsAsset'Locust_Grunt.PhysicsAsset.Locust_Grunt_CamSkel_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_Armor_Crotch","b_MF_Armor_Sho_R","b_MF_UpperArm_R","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")
	

	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'Locust_Grunt.Mesh.Locust_Grunt_CamSkel'
		PhysicsAsset=PhysicsAsset'Locust_Grunt.PhysicsAsset.Locust_Grunt_CamSkel_Physics'
	End Object

	Begin Object Name=CollisionCylinder
		CollisionRadius=+0034.000000
		CollisionHeight=+0072.000000
	End Object
}
