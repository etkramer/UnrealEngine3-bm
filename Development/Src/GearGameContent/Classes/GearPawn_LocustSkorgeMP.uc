/**
 *
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustSkorgeMP extends GearPawn_LocustKantusMP
	config(Pawn);


defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=66,V=127,UL=70,VL=62)
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_LocustKantus'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_LocustKantus"

	HeadSocketName="SkorgeHelmet"
    HelmetType=class'Item_Helmet_LocustSkorge'


	bUsingNewSoftWeightedGoreWhichHasStretchiesFixed=TRUE
	GoreSkeletalMesh=SkeletalMesh'Locust_Kantus.Locust_Skorge_MP_Gore'
	GorePhysicsAsset=PhysicsAsset'Locust_Kantus.PhysicsAsset.Locust_Skorge_MP_Physics'
	GoreBreakableJoints=("b_MF_Face","b_MF_Head","b_MF_Spine_03","b_MF_UpperArm_R","b_MF_UpperArm_L","b_MF_Hand_L","b_MF_Hand_R","b_MF_Calf_R","b_MF_Calf_L")


	Begin Object Name=GearPawnMesh
	    SkeletalMesh=SkeletalMesh'Locust_Kantus.Locust_Skorge_MP'
		PhysicsAsset=PhysicsAsset'Locust_Kantus.PhysicsAsset.Locust_Skorge_MP_Physics'
		AnimSets.Add(AnimSet'Locust_Kantus.Kantus_animset')
		Translation=(Z=-80)
		bEnableFullAnimWeightBodies=TRUE
	End Object

}