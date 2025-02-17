
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_LocustTheronWithHelmetMP extends GearPawn_LocustTheronMPBase
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_LocustHeads',U=0,V=0,UL=62,VL=62)
	HelmetType=class'Item_Helmet_LocustTheronThree'

	Begin Object Name=GearPawnMesh
		//SkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Mesh.Locust_Theron_Guard'
		//PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Physics'
		SkeletalMesh=SkeletalMesh'Locust_Theron_Guard.Mesh.Locust_Theron_Guard_Cloth'
		PhysicsAsset=PhysicsAsset'Locust_Theron_Guard.Locust_Theron_Guard_Cloth_Physics'
		Translation=(Z=-80)
	End Object
}