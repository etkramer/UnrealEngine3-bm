
/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGBaird extends GearPawn_COGGear
	config(Pawn);

defaultproperties
{
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=0,V=65,UL=48,VL=63)
	HelmetType=class'Item_Helmet_None'

	ControllerClass=class'GearAI_Baird'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGBaird'
	MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGBaird"
	NeedsRevivedGUDSEvent=GUDEvent_BairdNeedsRevived
	WentDownGUDSEvent=GUDEvent_BairdWentDown
	FAS_ChatterNames.Add("COG_Baird.FaceFX.Baird_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Baird.FaceFX.Baird_FaceFX_Efforts'

// AI CANNOT BE GORED IN SP - SAVES MEMORY
	GoreSkeletalMesh=none
	GorePhysicsAsset=none
	GoreBreakableJoints.Empty()
	HostageHealthBuckets.Empty()

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Baird.COG_Baird'
		PhysicsAsset=PhysicsAsset'COG_Baird.COG_Baird_Physics'
	End Object
}
