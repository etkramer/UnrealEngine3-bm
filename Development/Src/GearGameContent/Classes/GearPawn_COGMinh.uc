
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGMinh extends GearPawn_COGGear
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=98,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	ControllerClass=class'GearAI_Baird'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGMinh'
	//MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGMinh"			// minh is MP-only in Gears 2
	NeedsRevivedGUDSEvent=GUDEvent_MinhNeedsRevived
	WentDownGUDSEvent=GUDEvent_MinhWentDown
	FAS_ChatterNames.Add("COG_Minh.FaceFX.Chatter")

// AI CANNOT BE GORED IN SP - SAVES MEMORY
    GoreSkeletalMesh=none
	GorePhysicsAsset=none
	GoreBreakableJoints.Empty()
	HostageHealthBuckets.Empty()

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Minh.Minh_Kim'
		PhysicsAsset=PhysicsAsset'COG_Minh.Minh_Kim_Physics'
	End Object
}
