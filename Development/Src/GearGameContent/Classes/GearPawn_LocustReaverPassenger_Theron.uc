/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustReaverPassenger_Theron extends GearPawn_LocustReaverPassenger
	config(Pawn);



defaultproperties
{
	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGameContent.GearWeap_Bow'
	DefaultInventory(1)=class'GearGame.GearWeap_FragGrenade'
	DefaultInventory(2)=class'GearGame.GearWeap_LocustAssaultRifle'
	DefaultInventory(3)=class'GearGame.GearWeap_LocustPistol'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=245,V=0,UL=48,VL=63)
	HelmetType=class'Item_Helmet_LocustTheronRandom'

	ControllerClass=class'GearAI_Locust'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustTheron'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustTheron"

	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Locust_Theron_Guard_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
	GoreBreakableJoints=("b_MF_Forearm_L","b_MF_UpperArm_L","b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Spine_03","b_MF_Head","b_MF_Face","b_MF_Calf_L","b_MF_Calf_R","b_MF_Foot_R")
	HostageHealthBuckets=("b_MF_Hand_R","b_MF_UpperArm_R","b_MF_Calf_R")

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Mesh.Locust_Theron_Guard_Cloth'
		PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
	End Object

	PickupFocusBoneName=b_MF_Weapon_L
	PickupFocusBoneNameKickup=b_MF_Weapon_R

	SpecialMoveClasses(SM_MidLvlJumpOver)	=class'GSM_MantleOverLocust'
}
