
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustTheronMP extends GearPawn_LocustTheronMPBase
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=63,V=0,UL=62,VL=62)
	HelmetType=class'Item_Helmet_LocustTheronOne'

	Begin Object Name=GearPawnMesh
		//SkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Mesh.Locust_Theron_Guard'
		//PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Physics'
		SkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Mesh.Locust_Theron_Guard_Cloth'
		PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
		MorphSets.Add(MorphTargetSet'Locust_Theron_Guard.Mesh.Theron_Guard_Game_Morphs')
		Translation=(Z=-80)
	End Object
}