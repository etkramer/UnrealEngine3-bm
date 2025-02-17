
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGTai extends GearPawn_COGGear
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=407,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	ControllerClass=class'GearAI_Baird'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGTai'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGTai"
	NeedsRevivedGUDSEvent=GUDEvent_BairdNeedsRevived
	WentDownGUDSEvent=GUDEvent_BairdWentDown
	FAS_ChatterNames.Add("COG_Tai.FaceFX.Tai_FaceFX_Chatter") 
	FAS_Efforts(0)=FaceFXAnimSet'COG_Tai.FaceFX.Tai_FaceFX_Efforts'

// AI CANNOT BE GORED IN SP - SAVES MEMORY
    GoreSkeletalMesh=none
	GorePhysicsAsset=none
	GoreBreakableJoints.Empty()
	HostageHealthBuckets.Empty()

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Tai.COG_Tai'
		PhysicsAsset=PhysicsAsset'COG_Tai.COG_Tai_Physics'
	End Object

	DefaultInventory.Empty()
	DefaultInventory(0)=class'GearGame.GearWeap_Shotgun'
}
