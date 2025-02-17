
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustKantusMP extends GearPawn_LocustBaseMP;

simulated function vector GetWeaponAimIKPositionFix()
{
	local Vector	WeaponOffset;

	WeaponOffset = Super.GetWeaponAimIKPositionFix();
	WeaponOffset += vect(0,8,-8);
	return WeaponOffset;
}

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=0,V=127,UL=62,VL=62)

	ControllerClass=class'GearAI_Kantus'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustKantus'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustKantus"
	FAS_ChatterNames.Add("Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Chatter")
	FAS_ChatterNames.Add("Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Chatter_Dup")
	FAS_Efforts(0)=FaceFXAnimSet'Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Efforts'
	FAS_Efforts(1)=FaceFXAnimSet'Locust_Kantus.FaceFX.Locust_Kantus_FaceFX_Efforts_Dup'

	RimShaderMaterialSpecific=MaterialInstanceConstant'ALL_SoldierShaders.MultiPlayer.LOC_Kantus_Shader_MP'

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Kantus.Locust_Kantus_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Kantus.PhysicsAsset.Locust_Kantus_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")


	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Kantus.Locust_Kantus'
		PhysicsAsset=PhysicsAsset'Locust_Kantus.PhysicsAsset.Locust_Kantus_Physics'
		AnimSets.Add(AnimSet'Locust_Kantus.Kantus_animset')
		Translation=(Z=-80)
		bEnableFullAnimWeightBodies=TRUE
	End Object

	PickupFocusBoneName=b_MF_Weapon_L
	PickupFocusBoneNameKickup=b_MF_Weapon_R

	SpecialMoveClasses(SM_MidLvlJumpOver)			=class'GSM_MantleOverLocust'

}
