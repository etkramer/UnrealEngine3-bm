/**
 * Copyright 1998-2008 Epic Games, Inc. All Rights Reserved.
 */

class GearPawn_COGHoffman extends GearPawn_COGGear
	config(Pawn);

defaultproperties
{
	HelmetType=class'Item_Helmet_None'
	HeadIcon=(Texture=Texture2D'Warfare_HUD.HUD_Heads',U=0,V=0,UL=48,VL=63)

	ControllerClass=class'GearAI_Hoffman'
	SoundGroup=GearSoundGroup'GearSoundGroupArchetypes.GSG_COGHoffman'
	//MasterGUDBankClassNames(0)="GearGameContent.GUDData_COGHoffman"		// hoffman is scripted-only in sp
	NeedsRevivedGUDSEvent=GUDEvent_HoffmanNeedsRevived
	WentDownGUDSEvent=GUDEvent_HoffmanWentDown
	FAS_ChatterNames.Add("COG_Hoffman.FaceFX.COG_Hoffman_FaceFX_Chatter")
	FAS_Efforts(0)=FaceFXAnimSet'COG_Hoffman.FaceFX.COG_Hoffman_FaceFX_Efforts'

// AI CANNOT BE GORED IN SP - SAVES MEMORY
 	GoreSkeletalMesh=none
 	GorePhysicsAsset=none
 	GoreBreakableJoints.Empty()
 	HostageHealthBuckets.Empty()

	Begin Object Name=GearPawnMesh
		SkeletalMesh=SkeletalMesh'COG_Hoffman.COG_Hoffman'
		PhysicsAsset=PhysicsAsset'COG_Hoffman.COG_Hoffman_Physics' // tmp just use marcus's for now
	End Object
}
